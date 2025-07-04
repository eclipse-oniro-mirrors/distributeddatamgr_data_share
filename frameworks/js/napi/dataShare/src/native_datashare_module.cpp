/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "datashare_log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_datashare_const_properties.h"
#include "napi_datashare_helper.h"
#include "napi_dataproxy_handle.h"

namespace OHOS {
namespace DataShare {
/*
 * The module initialization.
 */
static napi_value Init(napi_env env, napi_value exports)
{
    LOG_DEBUG("Init DataShareHelper");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createDataShareHelper", NapiDataShareHelper::Napi_CreateDataShareHelper),
        DECLARE_NAPI_FUNCTION("enableSilentProxy", NapiDataShareHelper::EnableSilentProxy),
        DECLARE_NAPI_FUNCTION("disableSilentProxy", NapiDataShareHelper::DisableSilentProxy),
        DECLARE_NAPI_FUNCTION("createDataProxyHandle", NapiDataProxyHandle::Napi_CreateDataProxyHandle),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc);
    LOG_DEBUG("napi_define_properties status : %{public}d", status);
    status = InitConstProperties(env, exports);
    LOG_DEBUG("init Enumerate Constants %{public}d", status);
    return exports;
}

/*
 * The module definition.
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "data.dataShare",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * The module registration.
 */
static __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
}  // namespace DataShare
}  // namespace OHOS