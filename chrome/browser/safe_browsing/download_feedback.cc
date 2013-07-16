// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/safe_browsing/download_feedback.h"

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_util_proxy.h"
#include "base/metrics/histogram.h"
#include "base/task_runner.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/safe_browsing/csd.pb.h"
#include "net/base/net_errors.h"

namespace safe_browsing {

namespace {

// The default URL where browser sends download feedback requests.
const char kSbDefaultFeedbackURL[] =
    "https://safebrowsing.google.com/safebrowsing/uploads/chrome";

// This enum is used by histograms.  Do not change the ordering or remove items.
enum UploadResultType {
  UPLOAD_SUCCESS,
  UPLOAD_CANCELLED,
  UPLOAD_METADATA_NET_ERROR,
  UPLOAD_METADATA_RESPONSE_ERROR,
  UPLOAD_FILE_NET_ERROR,
  UPLOAD_FILE_RESPONSE_ERROR,
  UPLOAD_COMPLETE_RESPONSE_ERROR,
  // Memory space for histograms is determined by the max.
  // ALWAYS ADD NEW VALUES BEFORE THIS ONE.
  UPLOAD_RESULT_MAX
};

// Handles the uploading of a single downloaded binary to the safebrowsing
// download feedback service.
class DownloadFeedbackImpl : public DownloadFeedback {
 public:
  DownloadFeedbackImpl(net::URLRequestContextGetter* request_context_getter,
                       base::TaskRunner* file_task_runner,
                       const base::FilePath& file_path,
                       const std::string& ping_request,
                       const std::string& ping_response);
  virtual ~DownloadFeedbackImpl();

  virtual void Start(const base::Closure& finish_callback) OVERRIDE;

  virtual const std::string& GetPingRequestForTesting() const OVERRIDE {
    return ping_request_;
  }

  virtual const std::string& GetPingResponseForTesting() const OVERRIDE {
    return ping_response_;
  }

 private:
  // Callback for TwoPhaseUploader completion.  Relays the result to the
  // |finish_callback|.
  void FinishedUpload(base::Closure finish_callback,
                      TwoPhaseUploader::State state,
                      int net_error,
                      int response_code,
                      const std::string& response);

  void RecordUploadResult(UploadResultType result);

  scoped_refptr<net::URLRequestContextGetter> request_context_getter_;
  scoped_refptr<base::TaskRunner> file_task_runner_;
  const base::FilePath file_path_;
  int64 file_size_;

  // The safebrowsing request and response of checking that this binary is
  // unsafe.
  std::string ping_request_;
  std::string ping_response_;

  scoped_ptr<TwoPhaseUploader> uploader_;

  DISALLOW_COPY_AND_ASSIGN(DownloadFeedbackImpl);
};

DownloadFeedbackImpl::DownloadFeedbackImpl(
    net::URLRequestContextGetter* request_context_getter,
    base::TaskRunner* file_task_runner,
    const base::FilePath& file_path,
    const std::string& ping_request,
    const std::string& ping_response)
    : request_context_getter_(request_context_getter),
      file_task_runner_(file_task_runner),
      file_path_(file_path),
      file_size_(-1),
      ping_request_(ping_request),
      ping_response_(ping_response) {
  DVLOG(1) << "DownloadFeedback constructed " << this << " for "
           << file_path.AsUTF8Unsafe();
}

DownloadFeedbackImpl::~DownloadFeedbackImpl() {
  DCHECK(CalledOnValidThread());
  DVLOG(1) << "DownloadFeedback destructed " << this;

  if (uploader_) {
    RecordUploadResult(UPLOAD_CANCELLED);
    // Destroy the uploader before attempting to delete the file.
    uploader_.reset();
  }

  base::FileUtilProxy::DeleteFile(file_task_runner_.get(),
                                  file_path_,
                                  false,
                                  base::FileUtilProxy::StatusCallback());
}

void DownloadFeedbackImpl::Start(const base::Closure& finish_callback) {
  DCHECK(CalledOnValidThread());
  DCHECK(!uploader_);

  CommandLine* cmdline = CommandLine::ForCurrentProcess();

  ClientDownloadReport report_metadata;

  bool r = report_metadata.mutable_download_request()->ParseFromString(
      ping_request_);
  DCHECK(r);
  r = report_metadata.mutable_download_response()->ParseFromString(
      ping_response_);
  DCHECK(r);
  file_size_ = report_metadata.download_request().length();

  GURL url = GURL(cmdline->HasSwitch(switches::kSbDownloadFeedbackURL) ?
      cmdline->GetSwitchValueASCII(switches::kSbDownloadFeedbackURL) :
      kSbDefaultFeedbackURL);

  std::string metadata_string;
  bool ok = report_metadata.SerializeToString(&metadata_string);
  DCHECK(ok);
  uploader_.reset(
      TwoPhaseUploader::Create(request_context_getter_.get(),
                               file_task_runner_.get(),
                               url,
                               metadata_string,
                               file_path_,
                               TwoPhaseUploader::ProgressCallback(),
                               base::Bind(&DownloadFeedbackImpl::FinishedUpload,
                                          base::Unretained(this),
                                          finish_callback)));
  uploader_->Start();
}

void DownloadFeedbackImpl::FinishedUpload(base::Closure finish_callback,
                                          TwoPhaseUploader::State state,
                                          int net_error,
                                          int response_code,
                                          const std::string& response_data) {
  DCHECK(CalledOnValidThread());
  DVLOG(1) << __FUNCTION__ << " " << state << " rlen=" << response_data.size();

  switch (state) {
    case TwoPhaseUploader::STATE_SUCCESS: {
      ClientUploadResponse response;
      if (!response.ParseFromString(response_data) ||
          response.status() != ClientUploadResponse::SUCCESS)
        RecordUploadResult(UPLOAD_COMPLETE_RESPONSE_ERROR);
      else
        RecordUploadResult(UPLOAD_SUCCESS);
      break;
    }
    case TwoPhaseUploader::UPLOAD_FILE:
      if (net_error != net::OK)
        RecordUploadResult(UPLOAD_FILE_NET_ERROR);
      else
        RecordUploadResult(UPLOAD_FILE_RESPONSE_ERROR);
      break;
    case TwoPhaseUploader::UPLOAD_METADATA:
      if (net_error != net::OK)
        RecordUploadResult(UPLOAD_METADATA_NET_ERROR);
      else
        RecordUploadResult(UPLOAD_METADATA_RESPONSE_ERROR);
      break;
    default:
      NOTREACHED();
  }

  uploader_.reset();

  finish_callback.Run();
  // We may be deleted here.
}

void DownloadFeedbackImpl::RecordUploadResult(UploadResultType result) {
  if (result == UPLOAD_SUCCESS)
    UMA_HISTOGRAM_CUSTOM_COUNTS(
        "SBDownloadFeedback.SizeSuccess", file_size_, 1, kMaxUploadSize, 50);
  else
    UMA_HISTOGRAM_CUSTOM_COUNTS(
        "SBDownloadFeedback.SizeFailure", file_size_, 1, kMaxUploadSize, 50);
  UMA_HISTOGRAM_ENUMERATION(
      "SBDownloadFeedback.UploadResult", result, UPLOAD_RESULT_MAX);
}

}  // namespace

// static
const int64 DownloadFeedback::kMaxUploadSize = 50 * 1024 * 1024;

// static
DownloadFeedbackFactory* DownloadFeedback::factory_ = NULL;

// static
DownloadFeedback* DownloadFeedback::Create(
    net::URLRequestContextGetter* request_context_getter,
    base::TaskRunner* file_task_runner,
    const base::FilePath& file_path,
    const std::string& ping_request,
    const std::string& ping_response) {
  if (!DownloadFeedback::factory_)
    return new DownloadFeedbackImpl(
        request_context_getter, file_task_runner, file_path, ping_request,
        ping_response);
  return DownloadFeedback::factory_->CreateDownloadFeedback(
        request_context_getter, file_task_runner, file_path, ping_request,
        ping_response);
}

}  // namespace safe_browsing

