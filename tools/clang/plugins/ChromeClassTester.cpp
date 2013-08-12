// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// A general interface for filtering and only acting on classes in Chromium C++
// code.

#include "ChromeClassTester.h"

#include <sys/param.h>

#include "clang/AST/AST.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"

using namespace clang;

namespace {

bool starts_with(const std::string& one, const std::string& two) {
  return one.compare(0, two.size(), two) == 0;
}

std::string lstrip(const std::string& one, const std::string& two) {
  if (starts_with(one, two))
    return one.substr(two.size());
  return one;
}

bool ends_with(const std::string& one, const std::string& two) {
  if (two.size() > one.size())
    return false;

  return one.compare(one.size() - two.size(), two.size(), two) == 0;
}

}  // namespace

ChromeClassTester::ChromeClassTester(CompilerInstance& instance,
                                     bool check_url_directory)
    : instance_(instance),
      diagnostic_(instance.getDiagnostics()),
      check_url_directory_(check_url_directory) {
  BuildBannedLists();
}

ChromeClassTester::~ChromeClassTester() {}

void ChromeClassTester::HandleTagDeclDefinition(TagDecl* tag) {
  pending_class_decls_.push_back(tag);
}

bool ChromeClassTester::HandleTopLevelDecl(DeclGroupRef group_ref) {
  for (size_t i = 0; i < pending_class_decls_.size(); ++i)
    CheckTag(pending_class_decls_[i]);
  pending_class_decls_.clear();

  return true;  // true means continue parsing.
}

void ChromeClassTester::CheckTag(TagDecl* tag) {
  // We handle class types here where we have semantic information. We can only
  // check structs/classes/enums here, but we get a bunch of nice semantic
  // information instead of just parsing information.

  if (CXXRecordDecl* record = dyn_cast<CXXRecordDecl>(tag)) {
    // If this is a POD or a class template or a type dependent on a
    // templated class, assume there's no ctor/dtor/virtual method
    // optimization that we can do.
    if (record->isPOD() ||
        record->getDescribedClassTemplate() ||
        record->getTemplateSpecializationKind() ||
        record->isDependentType())
      return;

    if (InBannedNamespace(record))
      return;

    SourceLocation record_location = record->getInnerLocStart();
    if (InBannedDirectory(record_location))
      return;

    // We sadly need to maintain a blacklist of types that violate these
    // rules, but do so for good reason or due to limitations of this
    // checker (i.e., we don't handle extern templates very well).
    std::string base_name = record->getNameAsString();
    if (IsIgnoredType(base_name))
      return;

    // We ignore all classes that end with "Matcher" because they're probably
    // GMock artifacts.
    if (ends_with(base_name, "Matcher"))
        return;

    CheckChromeClass(record_location, record);
  }
}

void ChromeClassTester::emitWarning(SourceLocation loc,
                                    const char* raw_error) {
  FullSourceLoc full(loc, instance().getSourceManager());
  std::string err;
  err = "[chromium-style] ";
  err += raw_error;
  DiagnosticsEngine::Level level =
      diagnostic().getWarningsAsErrors() ?
      DiagnosticsEngine::Error :
      DiagnosticsEngine::Warning;
  unsigned id = diagnostic().getCustomDiagID(level, err);
  DiagnosticBuilder builder = diagnostic().Report(full, id);
}

bool ChromeClassTester::InBannedNamespace(const Decl* record) {
  std::string n = GetNamespace(record);
  if (!n.empty()) {
    return std::find(banned_namespaces_.begin(), banned_namespaces_.end(), n)
        != banned_namespaces_.end();
  }

  return false;
}

std::string ChromeClassTester::GetNamespace(const Decl* record) {
  return GetNamespaceImpl(record->getDeclContext(), "");
}

bool ChromeClassTester::InImplementationFile(SourceLocation record_location) {
  std::string filename;
  if (!GetFilename(record_location, &filename))
    return false;

  if (ends_with(filename, ".cc") || ends_with(filename, ".cpp") ||
      ends_with(filename, ".mm")) {
    return true;
  }

  return false;
}

void ChromeClassTester::BuildBannedLists() {
  banned_namespaces_.push_back("std");
  banned_namespaces_.push_back("__gnu_cxx");
  banned_namespaces_.push_back("WebKit");
  banned_namespaces_.push_back("WebTestRunner");

  banned_directories_.push_back("third_party/");
  banned_directories_.push_back("native_client/");
  banned_directories_.push_back("breakpad/");
  banned_directories_.push_back("courgette/");
  banned_directories_.push_back("pdf/");
  banned_directories_.push_back("ppapi/");
  banned_directories_.push_back("usr/");
  banned_directories_.push_back("testing/");
  banned_directories_.push_back("v8/");
  banned_directories_.push_back("dart/");
  banned_directories_.push_back("sdch/");
  banned_directories_.push_back("icu4c/");
  banned_directories_.push_back("frameworks/");

  if (!check_url_directory_)
    banned_directories_.push_back("url/");

  // Don't check autogenerated headers.
  // Make puts them below $(builddir_name)/.../gen and geni.
  // Ninja puts them below OUTPUT_DIR/.../gen
  // Xcode has a fixed output directory for everything.
  banned_directories_.push_back("gen/");
  banned_directories_.push_back("geni/");
  banned_directories_.push_back("xcodebuild/");

  // You are standing in a mazy of twisty dependencies, all resolved by
  // putting everything in the header.
  banned_directories_.push_back("automation/");

  // Don't check system headers.
  banned_directories_.push_back("/Developer/");

  // Used in really low level threading code that probably shouldn't be out of
  // lined.
  ignored_record_names_.insert("ThreadLocalBoolean");

  // A complicated pickle derived struct that is all packed integers.
  ignored_record_names_.insert("Header");

  // Part of the GPU system that uses multiple included header
  // weirdness. Never getting this right.
  ignored_record_names_.insert("Validators");

  // Has a UNIT_TEST only constructor. Isn't *terribly* complex...
  ignored_record_names_.insert("AutocompleteController");
  ignored_record_names_.insert("HistoryURLProvider");

  // Because of chrome frame
  ignored_record_names_.insert("ReliabilityTestSuite");

  // Used over in the net unittests. A large enough bundle of integers with 1
  // non-pod class member. Probably harmless.
  ignored_record_names_.insert("MockTransaction");

  // Used heavily in ui_unittests and once in views_unittests. Fixing this
  // isn't worth the overhead of an additional library.
  ignored_record_names_.insert("TestAnimationDelegate");

  // Part of our public interface that nacl and friends use. (Arguably, this
  // should mean that this is a higher priority but fixing this looks hard.)
  ignored_record_names_.insert("PluginVersionInfo");

  // Measured performance improvement on cc_perftests. See
  // https://codereview.chromium.org/11299290/
  ignored_record_names_.insert("QuadF");
}

std::string ChromeClassTester::GetNamespaceImpl(const DeclContext* context,
                                                const std::string& candidate) {
  switch (context->getDeclKind()) {
    case Decl::TranslationUnit: {
      return candidate;
    }
    case Decl::Namespace: {
      const NamespaceDecl* decl = dyn_cast<NamespaceDecl>(context);
      std::string name_str;
      llvm::raw_string_ostream OS(name_str);
      if (decl->isAnonymousNamespace())
        OS << "<anonymous namespace>";
      else
        OS << *decl;
      return GetNamespaceImpl(context->getParent(),
                              OS.str());
    }
    default: {
      return GetNamespaceImpl(context->getParent(), candidate);
    }
  }
}

bool ChromeClassTester::InBannedDirectory(SourceLocation loc) {
  std::string filename;
  if (!GetFilename(loc, &filename)) {
    // If the filename cannot be determined, simply treat this as a banned
    // location, instead of going through the full lookup process.
    return true;
  }

  // We need to special case scratch space; which is where clang does its
  // macro expansion. We explicitly want to allow people to do otherwise bad
  // things through macros that were defined due to third party libraries.
  if (filename == "<scratch space>")
    return true;

  // Don't complain about autogenerated protobuf files.
  if (ends_with(filename, ".pb.h")) {
    return true;
  }

  // We need to munge the paths so that they are relative to the repository
  // srcroot. We first resolve the symlinktastic relative path and then
  // remove our known srcroot from it if needed.
  char resolvedPath[MAXPATHLEN];
  if (realpath(filename.c_str(), resolvedPath)) {
    filename = resolvedPath;
  }

  // On linux, chrome is often checked out to /usr/local/google. Due to the
  // "usr" rule in banned_directories_, all diagnostics would be suppressed
  // in that case. As a workaround, strip that prefix.
  filename = lstrip(filename, "/usr/local/google");

  for (std::vector<std::string>::const_iterator it =
           banned_directories_.begin();
       it != banned_directories_.end(); ++it) {
    // If we can find any of the banned path components in this path, then
    // this file is rejected.
    size_t index = filename.find(*it);
    if (index != std::string::npos) {
      bool matches_full_dir_name = index == 0 || filename[index - 1] == '/';
      if ((*it)[0] == '/')
        matches_full_dir_name = true;
      if (matches_full_dir_name)
        return true;
    }
  }

  return false;
}

bool ChromeClassTester::IsIgnoredType(const std::string& base_name) {
  return ignored_record_names_.find(base_name) != ignored_record_names_.end();
}

bool ChromeClassTester::GetFilename(SourceLocation loc,
                                    std::string* filename) {
  const SourceManager& source_manager = instance_.getSourceManager();
  SourceLocation spelling_location = source_manager.getSpellingLoc(loc);
  PresumedLoc ploc = source_manager.getPresumedLoc(spelling_location);
  if (ploc.isInvalid()) {
    // If we're in an invalid location, we're looking at things that aren't
    // actually stated in the source.
    return false;
  }

  *filename = ploc.getFilename();
  return true;
}
