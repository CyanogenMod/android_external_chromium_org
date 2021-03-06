# This file is generated by gyp; do not edit.

include $(CLEAR_VARS)

LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE := ui_accessibility_ax_gen_gyp
LOCAL_MODULE_SUFFIX := .a
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_TARGET_ARCH := $(TARGET_$(GYP_VAR_PREFIX)ARCH)
gyp_intermediate_dir := $(call local-intermediates-dir,,$(GYP_VAR_PREFIX))
gyp_shared_intermediate_dir := $(call intermediates-dir-for,GYP,shared,,,$(GYP_VAR_PREFIX))

# Make sure our deps are built first.
GYP_TARGET_DEPENDENCIES :=

### Rules for action "genapi_bundle":
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h: gyp_local_path := $(LOCAL_PATH)
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h: gyp_var_prefix := $(GYP_VAR_PREFIX)
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h: gyp_intermediate_dir := $(abspath $(gyp_intermediate_dir))
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h: gyp_shared_intermediate_dir := $(abspath $(gyp_shared_intermediate_dir))
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h: export PATH := $(subst $(ANDROID_BUILD_PATHS),,$(PATH))
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h: $(LOCAL_PATH)/tools/json_schema_compiler/cc_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/code.py $(LOCAL_PATH)/tools/json_schema_compiler/compiler.py $(LOCAL_PATH)/tools/json_schema_compiler/cpp_bundle_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/cpp_type_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/cpp_util.py $(LOCAL_PATH)/tools/json_schema_compiler/h_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/idl_schema.py $(LOCAL_PATH)/tools/json_schema_compiler/json_schema.py $(LOCAL_PATH)/tools/json_schema_compiler/model.py $(LOCAL_PATH)/tools/json_schema_compiler/util_cc_helper.py $(LOCAL_PATH)/ui/accessibility/ax_enums.idl $(GYP_TARGET_DEPENDENCIES)
	@echo "Gyp action: Generating C++ API bundle code ($@)"
	$(hide)cd $(gyp_local_path)/ui/accessibility; mkdir -p $(gyp_shared_intermediate_dir)/ui/accessibility; python ../../tools/json_schema_compiler/compiler.py "--root=../.." "--destdir=$(gyp_shared_intermediate_dir)" "--namespace=" "--generator=cpp-bundle" "--impl-dir=chrome/browser/extensions/api" ax_enums.idl

$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.cc: $(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h ;
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_schemas.h: $(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h ;
$(gyp_shared_intermediate_dir)/ui/accessibility/generated_schemas.cc: $(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h ;



### Generated for rule "ui_accessibility_accessibility_gyp_ax_gen_target_genapi_idl":
# "{'inputs': ['../../tools/json_schema_compiler/cc_generator.py', '../../tools/json_schema_compiler/code.py', '../../tools/json_schema_compiler/compiler.py', '../../tools/json_schema_compiler/cpp_generator.py', '../../tools/json_schema_compiler/cpp_type_generator.py', '../../tools/json_schema_compiler/cpp_util.py', '../../tools/json_schema_compiler/h_generator.py', '../../tools/json_schema_compiler/idl_schema.py', '../../tools/json_schema_compiler/model.py', '../../tools/json_schema_compiler/util.cc', '../../tools/json_schema_compiler/util.h', '../../tools/json_schema_compiler/util_cc_helper.py'], 'process_outputs_as_sources': '1', 'extension': 'idl', 'msvs_external_rule': '1', 'outputs': ['$(gyp_shared_intermediate_dir)/ui/accessibility/%(INPUT_DIRNAME)s/%(INPUT_ROOT)s.cc', '$(gyp_shared_intermediate_dir)/ui/accessibility/%(INPUT_DIRNAME)s/%(INPUT_ROOT)s.h'], 'rule_name': 'genapi_idl', 'rule_sources': ['ax_enums.idl'], 'action': ['python', '../../tools/json_schema_compiler/compiler.py', '$(RULE_SOURCES)', '--root=../..', '--destdir=$(gyp_shared_intermediate_dir)', '--namespace=', '--generator=cpp', '--impl-dir=chrome/browser/extensions/api'], 'message': 'Generating C++ code from $(RULE_SOURCES) IDL files'}":
$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc: gyp_local_path := $(LOCAL_PATH)
$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc: gyp_var_prefix := $(GYP_VAR_PREFIX)
$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc: gyp_intermediate_dir := $(abspath $(gyp_intermediate_dir))
$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc: gyp_shared_intermediate_dir := $(abspath $(gyp_shared_intermediate_dir))
$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc: export PATH := $(subst $(ANDROID_BUILD_PATHS),,$(PATH))
$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc: $(LOCAL_PATH)/ui/accessibility/ax_enums.idl $(LOCAL_PATH)/tools/json_schema_compiler/cc_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/code.py $(LOCAL_PATH)/tools/json_schema_compiler/compiler.py $(LOCAL_PATH)/tools/json_schema_compiler/cpp_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/cpp_type_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/cpp_util.py $(LOCAL_PATH)/tools/json_schema_compiler/h_generator.py $(LOCAL_PATH)/tools/json_schema_compiler/idl_schema.py $(LOCAL_PATH)/tools/json_schema_compiler/model.py $(LOCAL_PATH)/tools/json_schema_compiler/util.cc $(LOCAL_PATH)/tools/json_schema_compiler/util.h $(LOCAL_PATH)/tools/json_schema_compiler/util_cc_helper.py $(GYP_TARGET_DEPENDENCIES)
	mkdir -p $(gyp_shared_intermediate_dir)/ui/accessibility; cd $(gyp_local_path)/ui/accessibility; python ../../tools/json_schema_compiler/compiler.py ax_enums.idl "--root=../.." "--destdir=$(gyp_shared_intermediate_dir)" "--namespace=" "--generator=cpp" "--impl-dir=chrome/browser/extensions/api"

$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.h: $(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc ;


GYP_GENERATED_OUTPUTS := \
	$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h \
	$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.cc \
	$(gyp_shared_intermediate_dir)/ui/accessibility/generated_schemas.h \
	$(gyp_shared_intermediate_dir)/ui/accessibility/generated_schemas.cc \
	$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc \
	$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.h

# Make sure our deps and generated files are built first.
LOCAL_ADDITIONAL_DEPENDENCIES := $(GYP_TARGET_DEPENDENCIES) $(GYP_GENERATED_OUTPUTS)

LOCAL_CPP_EXTENSION := .cc
$(gyp_intermediate_dir)/generated_api.cc: $(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.cc
	mkdir -p $(@D); cp $< $@
$(gyp_intermediate_dir)/generated_schemas.cc: $(gyp_shared_intermediate_dir)/ui/accessibility/generated_schemas.cc
	mkdir -p $(@D); cp $< $@
$(gyp_intermediate_dir)/ax_enums.cc: $(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.cc
	mkdir -p $(@D); cp $< $@
LOCAL_GENERATED_SOURCES := \
	$(gyp_intermediate_dir)/generated_api.cc \
	$(gyp_intermediate_dir)/generated_schemas.cc \
	$(gyp_intermediate_dir)/ax_enums.cc \
	$(gyp_shared_intermediate_dir)/ui/accessibility/generated_api.h \
	$(gyp_shared_intermediate_dir)/ui/accessibility/generated_schemas.h \
	$(gyp_shared_intermediate_dir)/ui/accessibility/ax_enums.h

GYP_COPIED_SOURCE_ORIGIN_DIRS := \
	$(gyp_shared_intermediate_dir)/ui/accessibility

LOCAL_SRC_FILES :=


# Flags passed to both C and C++ files.
MY_CFLAGS_Debug := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-Werror \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wall \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-unused-local-typedefs \
	-m64 \
	-march=x86-64 \
	-ffunction-sections \
	-funwind-tables \
	-g \
	-fstack-protector \
	-fno-short-enums \
	-finline-limit=64 \
	-Wa,--noexecstack \
	-U_FORTIFY_SOURCE \
	-Wno-extra \
	-Wno-ignored-qualifiers \
	-Wno-type-limits \
	-Wno-unused-but-set-variable \
	-Os \
	-g \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer \
	-funwind-tables

MY_DEFS_Debug := \
	'-DV8_DEPRECATION_WARNINGS' \
	'-DBLINK_SCALE_FILTERS_AT_RECORD_TIME' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_BROWSER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DDISCARDABLE_MEMORY_ALWAYS_SUPPORTED_NATIVELY' \
	'-DSYSTEM_NATIVELY_SIGNALS_MEMORY_PRESSURE' \
	'-DENABLE_EGLIMAGE=1' \
	'-DCLD_VERSION=1' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DDATA_REDUCTION_FALLBACK_HOST="http://compress.googlezip.net:80/"' \
	'-DDATA_REDUCTION_DEV_HOST="http://proxy-dev.googlezip.net:80/"' \
	'-DSPDY_PROXY_AUTH_ORIGIN="https://proxy.googlezip.net:443/"' \
	'-DDATA_REDUCTION_PROXY_PROBE_URL="http://check.googlezip.net/connect"' \
	'-DDATA_REDUCTION_PROXY_WARMUP_URL="http://www.gstatic.com/generate_204"' \
	'-DVIDEO_HOLE=1' \
	'-DUSE_OPENSSL=1' \
	'-DUSE_OPENSSL_CERTS=1' \
	'-D__STDC_CONSTANT_MACROS' \
	'-D__STDC_FORMAT_MACROS' \
	'-DANDROID' \
	'-D__GNU_SOURCE=1' \
	'-DUSE_STLPORT=1' \
	'-D_STLP_USE_PTR_SPECIALIZATIONS=1' \
	'-DCHROME_BUILD_ID=""' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=1' \
	'-DWTF_USE_DYNAMIC_ANNOTATIONS=1' \
	'-D_DEBUG'


# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES_Debug := \
	$(gyp_shared_intermediate_dir)/shim_headers/ashmem/target \
	$(gyp_shared_intermediate_dir) \
	$(LOCAL_PATH) \
	$(PWD)/frameworks/wilhelm/include \
	$(PWD)/bionic \
	$(PWD)/external/stlport/stlport


# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS_Debug := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wsign-compare \
	-Wno-non-virtual-dtor \
	-Wno-sign-promo


LOCAL_FDO_SUPPORT_Debug := false

# Flags passed to both C and C++ files.
MY_CFLAGS_Release := \
	-fstack-protector \
	--param=ssp-buffer-size=4 \
	-Werror \
	-fno-exceptions \
	-fno-strict-aliasing \
	-Wall \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-fvisibility=hidden \
	-pipe \
	-fPIC \
	-Wno-unused-local-typedefs \
	-m64 \
	-march=x86-64 \
	-ffunction-sections \
	-funwind-tables \
	-g \
	-fstack-protector \
	-fno-short-enums \
	-finline-limit=64 \
	-Wa,--noexecstack \
	-U_FORTIFY_SOURCE \
	-Wno-extra \
	-Wno-ignored-qualifiers \
	-Wno-type-limits \
	-Wno-unused-but-set-variable \
	-Os \
	-fno-ident \
	-fdata-sections \
	-ffunction-sections \
	-fomit-frame-pointer \
	-funwind-tables

MY_DEFS_Release := \
	'-DV8_DEPRECATION_WARNINGS' \
	'-DBLINK_SCALE_FILTERS_AT_RECORD_TIME' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DNO_TCMALLOC' \
	'-DDISABLE_NACL' \
	'-DCHROMIUM_BUILD' \
	'-DUSE_LIBJPEG_TURBO=1' \
	'-DENABLE_WEBRTC=1' \
	'-DUSE_PROPRIETARY_CODECS' \
	'-DENABLE_BROWSER_CDMS' \
	'-DENABLE_CONFIGURATION_POLICY' \
	'-DDISCARDABLE_MEMORY_ALWAYS_SUPPORTED_NATIVELY' \
	'-DSYSTEM_NATIVELY_SIGNALS_MEMORY_PRESSURE' \
	'-DENABLE_EGLIMAGE=1' \
	'-DCLD_VERSION=1' \
	'-DENABLE_PRINTING=1' \
	'-DENABLE_MANAGED_USERS=1' \
	'-DDATA_REDUCTION_FALLBACK_HOST="http://compress.googlezip.net:80/"' \
	'-DDATA_REDUCTION_DEV_HOST="http://proxy-dev.googlezip.net:80/"' \
	'-DSPDY_PROXY_AUTH_ORIGIN="https://proxy.googlezip.net:443/"' \
	'-DDATA_REDUCTION_PROXY_PROBE_URL="http://check.googlezip.net/connect"' \
	'-DDATA_REDUCTION_PROXY_WARMUP_URL="http://www.gstatic.com/generate_204"' \
	'-DVIDEO_HOLE=1' \
	'-DUSE_OPENSSL=1' \
	'-DUSE_OPENSSL_CERTS=1' \
	'-D__STDC_CONSTANT_MACROS' \
	'-D__STDC_FORMAT_MACROS' \
	'-DANDROID' \
	'-D__GNU_SOURCE=1' \
	'-DUSE_STLPORT=1' \
	'-D_STLP_USE_PTR_SPECIALIZATIONS=1' \
	'-DCHROME_BUILD_ID=""' \
	'-DNDEBUG' \
	'-DNVALGRIND' \
	'-DDYNAMIC_ANNOTATIONS_ENABLED=0' \
	'-D_FORTIFY_SOURCE=2'


# Include paths placed before CFLAGS/CPPFLAGS
LOCAL_C_INCLUDES_Release := \
	$(gyp_shared_intermediate_dir)/shim_headers/ashmem/target \
	$(gyp_shared_intermediate_dir) \
	$(LOCAL_PATH) \
	$(PWD)/frameworks/wilhelm/include \
	$(PWD)/bionic \
	$(PWD)/external/stlport/stlport


# Flags passed to only C++ (and not C) files.
LOCAL_CPPFLAGS_Release := \
	-fno-rtti \
	-fno-threadsafe-statics \
	-fvisibility-inlines-hidden \
	-Wsign-compare \
	-Wno-non-virtual-dtor \
	-Wno-sign-promo


LOCAL_FDO_SUPPORT_Release := false

LOCAL_CFLAGS := $(MY_CFLAGS_$(GYP_CONFIGURATION)) $(MY_DEFS_$(GYP_CONFIGURATION))
LOCAL_FDO_SUPPORT := $(LOCAL_FDO_SUPPORT_$(GYP_CONFIGURATION))
LOCAL_C_INCLUDES := $(GYP_COPIED_SOURCE_ORIGIN_DIRS) $(LOCAL_C_INCLUDES_$(GYP_CONFIGURATION))
LOCAL_CPPFLAGS := $(LOCAL_CPPFLAGS_$(GYP_CONFIGURATION))
LOCAL_ASFLAGS := $(LOCAL_CFLAGS)
### Rules for final target.

LOCAL_LDFLAGS_Debug := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-Wl,--fatal-warnings \
	-Wl,-z,noexecstack \
	-fPIC \
	-m64 \
	-fuse-ld=gold \
	-nostdlib \
	-Wl,--no-undefined \
	-Wl,--exclude-libs=ALL \
	-Wl,--warn-shared-textrel \
	-Wl,-O1 \
	-Wl,--as-needed


LOCAL_LDFLAGS_Release := \
	-Wl,-z,now \
	-Wl,-z,relro \
	-Wl,--fatal-warnings \
	-Wl,-z,noexecstack \
	-fPIC \
	-m64 \
	-fuse-ld=gold \
	-nostdlib \
	-Wl,--no-undefined \
	-Wl,--exclude-libs=ALL \
	-Wl,-O1 \
	-Wl,--as-needed \
	-Wl,--gc-sections \
	-Wl,--warn-shared-textrel


LOCAL_LDFLAGS := $(LOCAL_LDFLAGS_$(GYP_CONFIGURATION))

LOCAL_STATIC_LIBRARIES :=

# Enable grouping to fix circular references
LOCAL_GROUP_STATIC_LIBRARIES := true

LOCAL_SHARED_LIBRARIES := \
	libstlport \
	libdl

# Add target alias to "gyp_all_modules" target.
.PHONY: gyp_all_modules
gyp_all_modules: ui_accessibility_ax_gen_gyp

# Alias gyp target name.
.PHONY: ax_gen
ax_gen: ui_accessibility_ax_gen_gyp

include $(BUILD_STATIC_LIBRARY)
