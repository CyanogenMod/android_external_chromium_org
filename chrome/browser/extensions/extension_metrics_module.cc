// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_metrics_module.h"

#include "base/histogram.h"
#include "chrome/common/extensions/extension.h"
#include "chrome/browser/metrics/user_metrics.h"

namespace {

// Build the full name of a metrics for the given extension.  Each metric
// is made up of the unique name within the extension followed by the
// extension's id.  This keeps the metrics from one extension unique from
// other extensions, as well as those metrics from chrome itself.
std::string BuildMetricName(const std::string& name,
                            const Extension* extension) {
  std::string full_name(name);
  full_name += extension->id();
  return full_name;
}

}  // anonymous namespace

// These extension function classes are enabled only if the
// enable-metrics-extension-api command line switch is used.  Refer to
// extension_function_dispatcher.cc to see how they are enabled.

bool MetricsRecordUserActionFunction::RunImpl() {
  std::string name;
  EXTENSION_FUNCTION_VALIDATE(args_->IsType(Value::TYPE_STRING));
  EXTENSION_FUNCTION_VALIDATE(args_->GetAsString(&name));

  name = BuildMetricName(name, GetExtension());
  UserMetrics::RecordComputedAction(name, profile());
  return true;
}

bool MetricsHistogramHelperFunction::GetNameAndSample(std::string* name,
                                                      int* sample) {
  EXTENSION_FUNCTION_VALIDATE(args_->IsType(Value::TYPE_LIST));
  const ListValue* args = args_as_list();

  EXTENSION_FUNCTION_VALIDATE(args->GetString(0, name));
  EXTENSION_FUNCTION_VALIDATE(args->GetInteger(1, sample));
  return true;
}

bool MetricsHistogramHelperFunction::RecordValue(const std::string& name,
                                                 Histogram::ClassType type,
                                                 int min,
                                                 int max,
                                                 size_t buckets,
                                                 int sample) {
  std::string full_name = BuildMetricName(name, GetExtension());
  scoped_refptr<Histogram> counter;
  if (type == Histogram::LINEAR_HISTOGRAM) {
    counter = LinearHistogram::FactoryGet(full_name,
                                          min,
                                          max,
                                          buckets,
                                          Histogram::kUmaTargetedHistogramFlag);
  } else {
    counter = Histogram::FactoryGet(full_name,
                                    min,
                                    max,
                                    buckets,
                                    Histogram::kUmaTargetedHistogramFlag);
  }

  counter->Add(sample);
  return true;
}

bool MetricsRecordValueFunction::RunImpl() {
  EXTENSION_FUNCTION_VALIDATE(args_->IsType(Value::TYPE_LIST));
  const ListValue* args = args_as_list();

  int sample;
  EXTENSION_FUNCTION_VALIDATE(args->GetInteger(1, &sample));

  // Get the histogram parameters from the metric type object.
  DictionaryValue* metric_type;
  EXTENSION_FUNCTION_VALIDATE(args->GetDictionary(0, &metric_type));

  std::string name;
  std::string type;
  int min;
  int max;
  int buckets;
  EXTENSION_FUNCTION_VALIDATE(metric_type->GetString(L"metricName", &name));
  EXTENSION_FUNCTION_VALIDATE(metric_type->GetString(L"type", &type));
  EXTENSION_FUNCTION_VALIDATE(metric_type->GetInteger(L"min", &min));
  EXTENSION_FUNCTION_VALIDATE(metric_type->GetInteger(L"max", &max));
  EXTENSION_FUNCTION_VALIDATE(metric_type->GetInteger(L"buckets", &buckets));

  Histogram::ClassType histogram_type(type == "histogram-linear" ?
      Histogram::LINEAR_HISTOGRAM : Histogram::HISTOGRAM);
  return RecordValue(name, histogram_type, min, max, buckets, sample);
}

bool MetricsRecordPercentageFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  return RecordValue(name, Histogram::LINEAR_HISTOGRAM, 1, 101, 102, sample);
}

bool MetricsRecordCountFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  return RecordValue(name, Histogram::HISTOGRAM, 1, 1000000, 50, sample);
}

bool MetricsRecordSmallCountFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  return RecordValue(name, Histogram::HISTOGRAM, 1, 100, 50, sample);
}

bool MetricsRecordMediumCountFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  return RecordValue(name, Histogram::HISTOGRAM, 1, 10000, 50, sample);
}

bool MetricsRecordTimeFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  static const int kTenSecMs = 10 * 1000;
  return RecordValue(name, Histogram::HISTOGRAM, 1, kTenSecMs, 50, sample);
}

bool MetricsRecordMediumTimeFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  static const int kThreeMinMs = 3 * 60 * 1000;
  return RecordValue(name, Histogram::HISTOGRAM, 1, kThreeMinMs, 50, sample);
}

bool MetricsRecordLongTimeFunction::RunImpl() {
  std::string name;
  int sample;
  EXTENSION_FUNCTION_VALIDATE(GetNameAndSample(&name, &sample));
  static const int kOneHourMs = 60 * 60 * 1000;
  return RecordValue(name, Histogram::HISTOGRAM, 1, kOneHourMs, 50, sample);
}
