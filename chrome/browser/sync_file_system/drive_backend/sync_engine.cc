// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync_file_system/drive_backend/sync_engine.h"

#include <vector>

#include "base/bind.h"
#include "base/metrics/histogram.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/time/time.h"
#include "base/values.h"
#include "chrome/browser/drive/drive_api_service.h"
#include "chrome/browser/drive/drive_notification_manager.h"
#include "chrome/browser/drive/drive_notification_manager_factory.h"
#include "chrome/browser/drive/drive_service_interface.h"
#include "chrome/browser/drive/drive_uploader.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/signin/profile_oauth2_token_service_factory.h"
#include "chrome/browser/signin/signin_manager_factory.h"
#include "chrome/browser/sync_file_system/drive_backend/callback_helper.h"
#include "chrome/browser/sync_file_system/drive_backend/conflict_resolver.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_backend_constants.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_service_on_worker.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_service_wrapper.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_uploader_on_worker.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_uploader_wrapper.h"
#include "chrome/browser/sync_file_system/drive_backend/list_changes_task.h"
#include "chrome/browser/sync_file_system/drive_backend/local_to_remote_syncer.h"
#include "chrome/browser/sync_file_system/drive_backend/metadata_database.h"
#include "chrome/browser/sync_file_system/drive_backend/register_app_task.h"
#include "chrome/browser/sync_file_system/drive_backend/remote_change_processor_on_worker.h"
#include "chrome/browser/sync_file_system/drive_backend/remote_change_processor_wrapper.h"
#include "chrome/browser/sync_file_system/drive_backend/remote_to_local_syncer.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_engine_context.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_engine_initializer.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_task.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_worker.h"
#include "chrome/browser/sync_file_system/drive_backend/sync_worker_interface.h"
#include "chrome/browser/sync_file_system/drive_backend/uninstall_app_task.h"
#include "chrome/browser/sync_file_system/file_status_observer.h"
#include "chrome/browser/sync_file_system/logger.h"
#include "chrome/browser/sync_file_system/syncable_file_system_util.h"
#include "components/signin/core/browser/profile_oauth2_token_service.h"
#include "components/signin/core/browser/signin_manager.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/extension_system.h"
#include "extensions/browser/extension_system_provider.h"
#include "extensions/browser/extensions_browser_client.h"
#include "extensions/common/extension.h"
#include "google_apis/drive/drive_api_url_generator.h"
#include "google_apis/drive/gdata_wapi_url_generator.h"
#include "net/url_request/url_request_context_getter.h"
#include "webkit/common/blob/scoped_file.h"
#include "webkit/common/fileapi/file_system_util.h"

namespace sync_file_system {

class RemoteChangeProcessor;

namespace drive_backend {

class SyncEngine::WorkerObserver : public SyncWorkerInterface::Observer {
 public:
  WorkerObserver(base::SequencedTaskRunner* ui_task_runner,
                 base::WeakPtr<SyncEngine> sync_engine)
      : ui_task_runner_(ui_task_runner),
        sync_engine_(sync_engine) {
    sequence_checker_.DetachFromSequence();
  }

  virtual ~WorkerObserver() {
    DCHECK(sequence_checker_.CalledOnValidSequencedThread());
  }

  virtual void OnPendingFileListUpdated(int item_count) OVERRIDE {
    if (ui_task_runner_->RunsTasksOnCurrentThread()) {
      if (sync_engine_)
        sync_engine_->OnPendingFileListUpdated(item_count);
      return;
    }

    DCHECK(sequence_checker_.CalledOnValidSequencedThread());
    ui_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&SyncEngine::OnPendingFileListUpdated,
                   sync_engine_,
                   item_count));
  }

  virtual void OnFileStatusChanged(const fileapi::FileSystemURL& url,
                                   SyncFileStatus file_status,
                                   SyncAction sync_action,
                                   SyncDirection direction) OVERRIDE {
    if (ui_task_runner_->RunsTasksOnCurrentThread()) {
      if (sync_engine_)
        sync_engine_->OnFileStatusChanged(
            url, file_status, sync_action, direction);
      return;
    }

    DCHECK(sequence_checker_.CalledOnValidSequencedThread());
    ui_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&SyncEngine::OnFileStatusChanged,
                   sync_engine_,
                   url, file_status, sync_action, direction));
  }

  virtual void UpdateServiceState(RemoteServiceState state,
                                  const std::string& description) OVERRIDE {
    if (ui_task_runner_->RunsTasksOnCurrentThread()) {
      if (sync_engine_)
        sync_engine_->UpdateServiceState(state, description);
      return;
    }

    DCHECK(sequence_checker_.CalledOnValidSequencedThread());
    ui_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&SyncEngine::UpdateServiceState,
                   sync_engine_, state, description));
  }

  void DetachFromSequence() {
    sequence_checker_.DetachFromSequence();
  }

 private:
  scoped_refptr<base::SequencedTaskRunner> ui_task_runner_;
  base::WeakPtr<SyncEngine> sync_engine_;

  base::SequenceChecker sequence_checker_;

  DISALLOW_COPY_AND_ASSIGN(WorkerObserver);
};

namespace {

void DidRegisterOrigin(const base::TimeTicks& start_time,
                       const SyncStatusCallback& callback,
                       SyncStatusCode status) {
  base::TimeDelta delta(base::TimeTicks::Now() - start_time);
  HISTOGRAM_TIMES("SyncFileSystem.RegisterOriginTime", delta);
  callback.Run(status);
}

template <typename T>
void DeleteSoonHelper(scoped_ptr<T>) {}

template <typename T>
void DeleteSoon(const tracked_objects::Location& from_here,
                base::TaskRunner* task_runner,
                scoped_ptr<T> obj) {
  if (!obj)
    return;

  T* obj_ptr = obj.get();
  base::Closure deleter =
      base::Bind(&DeleteSoonHelper<T>, base::Passed(&obj));
  if (!task_runner->PostTask(from_here, deleter)) {
    obj_ptr->DetachFromSequence();
    deleter.Run();
  }
}

}  // namespace

scoped_ptr<SyncEngine> SyncEngine::CreateForBrowserContext(
    content::BrowserContext* context,
    TaskLogger* task_logger) {
  scoped_refptr<base::SequencedWorkerPool> worker_pool =
      content::BrowserThread::GetBlockingPool();

  scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner =
      base::MessageLoopProxy::current();
  scoped_refptr<base::SequencedTaskRunner> worker_task_runner =
      worker_pool->GetSequencedTaskRunnerWithShutdownBehavior(
          worker_pool->GetSequenceToken(),
          base::SequencedWorkerPool::SKIP_ON_SHUTDOWN);
  scoped_refptr<base::SequencedTaskRunner> file_task_runner =
      worker_pool->GetSequencedTaskRunnerWithShutdownBehavior(
          worker_pool->GetSequenceToken(),
          base::SequencedWorkerPool::SKIP_ON_SHUTDOWN);
  scoped_refptr<base::SequencedTaskRunner> drive_task_runner =
      worker_pool->GetSequencedTaskRunnerWithShutdownBehavior(
          worker_pool->GetSequenceToken(),
          base::SequencedWorkerPool::SKIP_ON_SHUTDOWN);

  Profile* profile = Profile::FromBrowserContext(context);
  drive::DriveNotificationManager* notification_manager =
      drive::DriveNotificationManagerFactory::GetForBrowserContext(context);
  ExtensionService* extension_service =
      extensions::ExtensionSystem::Get(context)->extension_service();
  SigninManagerBase* signin_manager =
      SigninManagerFactory::GetForProfile(profile);
  ProfileOAuth2TokenService* token_service =
      ProfileOAuth2TokenServiceFactory::GetForProfile(profile);
  scoped_refptr<net::URLRequestContextGetter> request_context =
      context->GetRequestContext();

  scoped_ptr<drive_backend::SyncEngine> sync_engine(
      new SyncEngine(ui_task_runner,
                     worker_task_runner,
                     file_task_runner,
                     drive_task_runner,
                     GetSyncFileSystemDir(context->GetPath()),
                     task_logger,
                     notification_manager,
                     extension_service,
                     signin_manager,
                     token_service,
                     request_context,
                     NULL  /* env_override */));

  sync_engine->Initialize();
  return sync_engine.Pass();
}

void SyncEngine::AppendDependsOnFactories(
    std::set<BrowserContextKeyedServiceFactory*>* factories) {
  DCHECK(factories);
  factories->insert(drive::DriveNotificationManagerFactory::GetInstance());
  factories->insert(SigninManagerFactory::GetInstance());
  factories->insert(
      extensions::ExtensionsBrowserClient::Get()->GetExtensionSystemFactory());
  factories->insert(ProfileOAuth2TokenServiceFactory::GetInstance());
}

SyncEngine::~SyncEngine() {
  Reset();

  net::NetworkChangeNotifier::RemoveNetworkChangeObserver(this);
  if (signin_manager_)
    signin_manager_->RemoveObserver(this);
  if (notification_manager_)
    notification_manager_->RemoveObserver(this);
}

void SyncEngine::Reset() {
  if (drive_service_)
    drive_service_->RemoveObserver(this);

  DeleteSoon(FROM_HERE, worker_task_runner_, sync_worker_.Pass());
  DeleteSoon(FROM_HERE, worker_task_runner_, worker_observer_.Pass());
  DeleteSoon(FROM_HERE, worker_task_runner_,
             remote_change_processor_on_worker_.Pass());

  drive_service_wrapper_.reset();
  drive_service_.reset();
  drive_uploader_wrapper_.reset();
  drive_uploader_.reset();
  remote_change_processor_wrapper_.reset();
  callback_tracker_.AbortAll();
}

void SyncEngine::Initialize() {
  Reset();

  if (!signin_manager_ ||
      signin_manager_->GetAuthenticatedAccountId().empty())
    return;

  scoped_ptr<drive::DriveServiceInterface> drive_service(
      new drive::DriveAPIService(
          token_service_,
          request_context_,
          drive_task_runner_,
          GURL(google_apis::DriveApiUrlGenerator::kBaseUrlForProduction),
          GURL(google_apis::DriveApiUrlGenerator::
               kBaseDownloadUrlForProduction),
          GURL(google_apis::GDataWapiUrlGenerator::kBaseUrlForProduction),
          std::string() /* custom_user_agent */));
  scoped_ptr<drive::DriveUploaderInterface> drive_uploader(
      new drive::DriveUploader(drive_service.get(), drive_task_runner_));

  InitializeInternal(drive_service.Pass(), drive_uploader.Pass(),
                     scoped_ptr<SyncWorkerInterface>());
}

void SyncEngine::InitializeForTesting(
    scoped_ptr<drive::DriveServiceInterface> drive_service,
    scoped_ptr<drive::DriveUploaderInterface> drive_uploader,
    scoped_ptr<SyncWorkerInterface> sync_worker) {
  Reset();
  InitializeInternal(drive_service.Pass(), drive_uploader.Pass(),
                     sync_worker.Pass());
}

void SyncEngine::InitializeInternal(
    scoped_ptr<drive::DriveServiceInterface> drive_service,
    scoped_ptr<drive::DriveUploaderInterface> drive_uploader,
    scoped_ptr<SyncWorkerInterface> sync_worker) {
  drive_service_ = drive_service.Pass();
  drive_service_wrapper_.reset(new DriveServiceWrapper(drive_service_.get()));

  std::string account_id;
  if (signin_manager_)
    account_id = signin_manager_->GetAuthenticatedAccountId();
  drive_service_->Initialize(account_id);

  drive_uploader_ = drive_uploader.Pass();
  drive_uploader_wrapper_.reset(
      new DriveUploaderWrapper(drive_uploader_.get()));

  // DriveServiceWrapper and DriveServiceOnWorker relay communications
  // between DriveService and syncers in SyncWorker.
  scoped_ptr<drive::DriveServiceInterface> drive_service_on_worker(
      new DriveServiceOnWorker(drive_service_wrapper_->AsWeakPtr(),
                               ui_task_runner_,
                               worker_task_runner_));
  scoped_ptr<drive::DriveUploaderInterface> drive_uploader_on_worker(
      new DriveUploaderOnWorker(drive_uploader_wrapper_->AsWeakPtr(),
                                ui_task_runner_,
                                worker_task_runner_));
  scoped_ptr<SyncEngineContext> sync_engine_context(
      new SyncEngineContext(drive_service_on_worker.Pass(),
                            drive_uploader_on_worker.Pass(),
                            task_logger_,
                            ui_task_runner_,
                            worker_task_runner_,
                            file_task_runner_));

  worker_observer_.reset(
      new WorkerObserver(ui_task_runner_, weak_ptr_factory_.GetWeakPtr()));

  base::WeakPtr<ExtensionServiceInterface> extension_service_weak_ptr;
  if (extension_service_)
    extension_service_weak_ptr = extension_service_->AsWeakPtr();

  if (!sync_worker) {
    sync_worker.reset(new SyncWorker(
        sync_file_system_dir_,
        extension_service_weak_ptr,
        env_override_));
  }

  sync_worker_ = sync_worker.Pass();
  sync_worker_->AddObserver(worker_observer_.get());

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::Initialize,
                 base::Unretained(sync_worker_.get()),
                 base::Passed(&sync_engine_context)));
  if (remote_change_processor_)
    SetRemoteChangeProcessor(remote_change_processor_);

  drive_service_->AddObserver(this);

  service_state_ = REMOTE_SERVICE_TEMPORARY_UNAVAILABLE;
  SetSyncEnabled(sync_enabled_);
  OnNetworkChanged(net::NetworkChangeNotifier::GetConnectionType());
}

void SyncEngine::AddServiceObserver(SyncServiceObserver* observer) {
  service_observers_.AddObserver(observer);
}

void SyncEngine::AddFileStatusObserver(FileStatusObserver* observer) {
  file_status_observers_.AddObserver(observer);
}

void SyncEngine::RegisterOrigin(const GURL& origin,
                                const SyncStatusCallback& callback) {
  if (!sync_worker_) {
    // TODO(tzik): Record |origin| and retry the registration after late
    // sign-in.  Then, return SYNC_STATUS_OK.
    if (!signin_manager_ ||
        signin_manager_->GetAuthenticatedAccountId().empty())
      callback.Run(SYNC_STATUS_AUTHENTICATION_FAILED);
    else
      callback.Run(SYNC_STATUS_ABORT);
    return;
  }

  SyncStatusCallback relayed_callback = RelayCallbackToCurrentThread(
      FROM_HERE, base::Bind(&DidRegisterOrigin, base::TimeTicks::Now(),
                            TrackCallback(callback)));

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::RegisterOrigin,
                 base::Unretained(sync_worker_.get()),
                 origin, relayed_callback));
}

void SyncEngine::EnableOrigin(
    const GURL& origin, const SyncStatusCallback& callback) {
  if (!sync_worker_) {
    // It's safe to return OK immediately since this is also checked in
    // SyncWorker initialization.
    callback.Run(SYNC_STATUS_OK);
    return;
  }

  SyncStatusCallback relayed_callback = RelayCallbackToCurrentThread(
      FROM_HERE, TrackCallback(callback));

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::EnableOrigin,
                 base::Unretained(sync_worker_.get()),
                 origin, relayed_callback));
}

void SyncEngine::DisableOrigin(
    const GURL& origin, const SyncStatusCallback& callback) {
  if (!sync_worker_) {
    // It's safe to return OK immediately since this is also checked in
    // SyncWorker initialization.
    callback.Run(SYNC_STATUS_OK);
    return;
  }

  SyncStatusCallback relayed_callback = RelayCallbackToCurrentThread(
      FROM_HERE, TrackCallback(callback));

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::DisableOrigin,
                 base::Unretained(sync_worker_.get()),
                 origin,
                 relayed_callback));
}

void SyncEngine::UninstallOrigin(
    const GURL& origin,
    UninstallFlag flag,
    const SyncStatusCallback& callback) {
  if (!sync_worker_) {
    // It's safe to return OK immediately since this is also checked in
    // SyncWorker initialization.
    callback.Run(SYNC_STATUS_OK);
    return;
  }

  SyncStatusCallback relayed_callback = RelayCallbackToCurrentThread(
      FROM_HERE, TrackCallback(callback));
  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::UninstallOrigin,
                 base::Unretained(sync_worker_.get()),
                 origin, flag, relayed_callback));
}

void SyncEngine::ProcessRemoteChange(const SyncFileCallback& callback) {
  base::Closure abort_closure =
      base::Bind(callback, SYNC_STATUS_ABORT, fileapi::FileSystemURL());

  if (!sync_worker_) {
    abort_closure.Run();
    return;
  }

  SyncFileCallback tracked_callback = callback_tracker_.Register(
      abort_closure, callback);
  SyncFileCallback relayed_callback = RelayCallbackToCurrentThread(
      FROM_HERE, tracked_callback);
  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::ProcessRemoteChange,
                 base::Unretained(sync_worker_.get()),
                 relayed_callback));
}

void SyncEngine::SetRemoteChangeProcessor(RemoteChangeProcessor* processor) {
  remote_change_processor_ = processor;

  if (!sync_worker_)
    return;

  remote_change_processor_wrapper_.reset(
      new RemoteChangeProcessorWrapper(processor));

  remote_change_processor_on_worker_.reset(new RemoteChangeProcessorOnWorker(
      remote_change_processor_wrapper_->AsWeakPtr(),
      ui_task_runner_, worker_task_runner_));

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::SetRemoteChangeProcessor,
                 base::Unretained(sync_worker_.get()),
                 remote_change_processor_on_worker_.get()));
}

LocalChangeProcessor* SyncEngine::GetLocalChangeProcessor() {
  return this;
}

RemoteServiceState SyncEngine::GetCurrentState() const {
  return service_state_;
}

void SyncEngine::GetOriginStatusMap(const StatusMapCallback& callback) {
  base::Closure abort_closure =
      base::Bind(callback, base::Passed(scoped_ptr<OriginStatusMap>()));

  if (!sync_worker_) {
    abort_closure.Run();
    return;
  }

  StatusMapCallback tracked_callback =
      callback_tracker_.Register(abort_closure, callback);
  StatusMapCallback relayed_callback =
      RelayCallbackToCurrentThread(FROM_HERE, tracked_callback);

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::GetOriginStatusMap,
                 base::Unretained(sync_worker_.get()),
                 relayed_callback));
}

void SyncEngine::DumpFiles(const GURL& origin,
                           const ListCallback& callback) {
  base::Closure abort_closure =
      base::Bind(callback, base::Passed(scoped_ptr<base::ListValue>()));

  if (!sync_worker_) {
    abort_closure.Run();
    return;
  }

  ListCallback tracked_callback =
      callback_tracker_.Register(abort_closure, callback);

  PostTaskAndReplyWithResult(
      worker_task_runner_,
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::DumpFiles,
                 base::Unretained(sync_worker_.get()),
                 origin),
      tracked_callback);
}

void SyncEngine::DumpDatabase(const ListCallback& callback) {
  base::Closure abort_closure =
      base::Bind(callback, base::Passed(scoped_ptr<base::ListValue>()));

  if (!sync_worker_) {
    abort_closure.Run();
    return;
  }

  ListCallback tracked_callback =
      callback_tracker_.Register(abort_closure, callback);

  PostTaskAndReplyWithResult(
      worker_task_runner_,
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::DumpDatabase,
                 base::Unretained(sync_worker_.get())),
      tracked_callback);
}

void SyncEngine::SetSyncEnabled(bool sync_enabled) {
  sync_enabled_ = sync_enabled;

  if (!sync_worker_)
    return;

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::SetSyncEnabled,
                 base::Unretained(sync_worker_.get()),
                 sync_enabled));
}

void SyncEngine::PromoteDemotedChanges() {
  if (!sync_worker_)
    return;

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::PromoteDemotedChanges,
                 base::Unretained(sync_worker_.get())));
}

void SyncEngine::ApplyLocalChange(
    const FileChange& local_change,
    const base::FilePath& local_path,
    const SyncFileMetadata& local_metadata,
    const fileapi::FileSystemURL& url,
    const SyncStatusCallback& callback) {
  if (!sync_worker_) {
    callback.Run(SYNC_STATUS_ABORT);
    return;
  }

  SyncStatusCallback relayed_callback = RelayCallbackToCurrentThread(
      FROM_HERE, TrackCallback(callback));
  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::ApplyLocalChange,
                 base::Unretained(sync_worker_.get()),
                 local_change,
                 local_path,
                 local_metadata,
                 url,
                 relayed_callback));
}

void SyncEngine::OnNotificationReceived() {
  if (!sync_worker_)
    return;

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::OnNotificationReceived,
                 base::Unretained(sync_worker_.get())));
}

void SyncEngine::OnPushNotificationEnabled(bool) {}

void SyncEngine::OnReadyToSendRequests() {
  if (!sync_worker_)
    return;

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::OnReadyToSendRequests,
                 base::Unretained(sync_worker_.get())));
}

void SyncEngine::OnRefreshTokenInvalid() {
  if (!sync_worker_)
    return;

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::OnRefreshTokenInvalid,
                 base::Unretained(sync_worker_.get())));
}

void SyncEngine::OnNetworkChanged(
    net::NetworkChangeNotifier::ConnectionType type) {
  if (!sync_worker_)
    return;

  worker_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&SyncWorkerInterface::OnNetworkChanged,
                 base::Unretained(sync_worker_.get()),
                 type));
}

void SyncEngine::GoogleSigninFailed(const GoogleServiceAuthError& error) {
  Reset();
  UpdateServiceState(REMOTE_SERVICE_AUTHENTICATION_REQUIRED,
                     "Failed to sign in.");
}

void SyncEngine::GoogleSigninSucceeded(const std::string& username,
                                       const std::string& password) {
  Initialize();
}

void SyncEngine::GoogleSignedOut(const std::string& username) {
  Reset();
  UpdateServiceState(REMOTE_SERVICE_AUTHENTICATION_REQUIRED,
                     "User signed out.");
}

SyncEngine::SyncEngine(
    base::SingleThreadTaskRunner* ui_task_runner,
    base::SequencedTaskRunner* worker_task_runner,
    base::SequencedTaskRunner* file_task_runner,
    base::SequencedTaskRunner* drive_task_runner,
    const base::FilePath& sync_file_system_dir,
    TaskLogger* task_logger,
    drive::DriveNotificationManager* notification_manager,
    ExtensionServiceInterface* extension_service,
    SigninManagerBase* signin_manager,
    ProfileOAuth2TokenService* token_service,
    net::URLRequestContextGetter* request_context,
    leveldb::Env* env_override)
    : ui_task_runner_(ui_task_runner),
      worker_task_runner_(worker_task_runner),
      file_task_runner_(file_task_runner),
      drive_task_runner_(drive_task_runner),
      sync_file_system_dir_(sync_file_system_dir),
      task_logger_(task_logger),
      notification_manager_(notification_manager),
      extension_service_(extension_service),
      signin_manager_(signin_manager),
      token_service_(token_service),
      request_context_(request_context),
      remote_change_processor_(NULL),
      service_state_(REMOTE_SERVICE_TEMPORARY_UNAVAILABLE),
      sync_enabled_(false),
      env_override_(env_override),
      weak_ptr_factory_(this) {
  DCHECK(sync_file_system_dir_.IsAbsolute());
  if (notification_manager_)
    notification_manager_->AddObserver(this);
  if (signin_manager_)
    signin_manager_->AddObserver(this);
  net::NetworkChangeNotifier::AddNetworkChangeObserver(this);
}

void SyncEngine::OnPendingFileListUpdated(int item_count) {
  FOR_EACH_OBSERVER(
      SyncServiceObserver,
      service_observers_,
      OnRemoteChangeQueueUpdated(item_count));
}

void SyncEngine::OnFileStatusChanged(const fileapi::FileSystemURL& url,
                                     SyncFileStatus file_status,
                                     SyncAction sync_action,
                                     SyncDirection direction) {
  FOR_EACH_OBSERVER(FileStatusObserver,
                    file_status_observers_,
                    OnFileStatusChanged(
                        url, file_status, sync_action, direction));
}

void SyncEngine::UpdateServiceState(RemoteServiceState state,
                                    const std::string& description) {
  service_state_ = state;

  FOR_EACH_OBSERVER(
      SyncServiceObserver, service_observers_,
      OnRemoteServiceStateUpdated(state, description));
}

SyncStatusCallback SyncEngine::TrackCallback(
    const SyncStatusCallback& callback) {
  return callback_tracker_.Register(
      base::Bind(callback, SYNC_STATUS_ABORT),
      callback);
}

}  // namespace drive_backend
}  // namespace sync_file_system
