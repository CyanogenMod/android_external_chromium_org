// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SIGNIN_SIGNIN_MANAGER_FACTORY_H_
#define CHROME_BROWSER_SIGNIN_SIGNIN_MANAGER_FACTORY_H_

#include "base/memory/singleton.h"
#include "base/observer_list.h"
#include "components/browser_context_keyed_service/browser_context_keyed_service_factory.h"

class SigninManager;
class SigninManagerBase;
class PrefRegistrySimple;
class Profile;

// Singleton that owns all SigninManagers and associates them with
// Profiles. Listens for the Profile's destruction notification and cleans up
// the associated SigninManager.
class SigninManagerFactory : public BrowserContextKeyedServiceFactory {
 public:
  class Observer {
   public:
    // Called when a SigninManager(Base) instance is created.
    virtual void SigninManagerCreated(SigninManagerBase* manager) {}

    // Called when a SigninManager(Base) instance is being shut down. Observers
    // of |manager| should remove themselves at this point.
    virtual void SigninManagerShutdown(SigninManagerBase* manager) {}

   protected:
    virtual ~Observer() {}
  };

#if defined(OS_CHROMEOS)
  // Returns the instance of SigninManager associated with this profile
  // (creating one if none exists). Returns NULL if this profile cannot have a
  // SigninManager (for example, if |profile| is incognito).
  static SigninManagerBase* GetForProfile(Profile* profile);

  // Returns the instance of SigninManager associated with this profile. Returns
  // null if no SigninManager instance currently exists (will not create a new
  // instance).
  static SigninManagerBase* GetForProfileIfExists(Profile* profile);
#else
  // On non-ChromeOS platforms, the SigninManager the factory creates will be
  // an instance of the extended SigninManager class.
  static SigninManager* GetForProfile(Profile* profile);
  static SigninManager* GetForProfileIfExists(Profile* profile);
#endif

  // Returns an instance of the SigninManagerFactory singleton.
  static SigninManagerFactory* GetInstance();

  // Implementation of BrowserContextKeyedServiceFactory (public so tests
  // can call it).
  virtual void RegisterProfilePrefs(
      user_prefs::PrefRegistrySyncable* registry) OVERRIDE;

  // Registers the browser-global prefs used by SigninManager.
  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Methods to register or remove observers of SigninManager creation/shutdown.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  // Notifies observers of |manager|'s creation. Should be called only by test
  // SigninManager subclasses whose construction does not occur in
  // |BuildServiceInstanceFor()|.
  void NotifyObserversOfSigninManagerCreationForTesting(
      SigninManagerBase* manager);

 private:
  friend struct DefaultSingletonTraits<SigninManagerFactory>;

  SigninManagerFactory();
  virtual ~SigninManagerFactory();

#if defined(OS_MACOSX)
  // List of observers. Does not check that list is empty on destruction, as
  // there are some leaky singletons that observe the SigninManagerFactory.
  mutable ObserverList<Observer> observer_list_;
#else
  // List of observers. Checks that list is empty on destruction.
  mutable ObserverList<Observer, true> observer_list_;
#endif

  // BrowserContextKeyedServiceFactory:
  virtual BrowserContextKeyedService* BuildServiceInstanceFor(
      content::BrowserContext* profile) const OVERRIDE;
  virtual void BrowserContextShutdown(content::BrowserContext* context)
      OVERRIDE;
};

#endif  // CHROME_BROWSER_SIGNIN_SIGNIN_MANAGER_FACTORY_H_
