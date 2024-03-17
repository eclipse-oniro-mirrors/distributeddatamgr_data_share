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

/* SAID:1301 */
/* FeatureSystem: data_share_service */
#ifndef DISTRIBUTEDDATA_DATA_SHARE_IPC_INTERFACE_H
#define DISTRIBUTEDDATA_DATA_SHARE_IPC_INTERFACE_H

namespace OHOS::DistributedShare {
namespace DataShare {
enum class IDataShareInterfaceCode {
    CMD_GET_FILE_TYPES = 1,
    CMD_OPEN_FILE,
    CMD_OPEN_RAW_FILE,
    CMD_INSERT,
    CMD_UPDATE,
    CMD_DELETE,
    CMD_QUERY,
    CMD_GET_TYPE,
    CMD_BATCH_INSERT,
    CMD_REGISTER_OBSERVER,
    CMD_UNREGISTER_OBSERVER,
    CMD_NOTIFY_CHANGE,
    CMD_NORMALIZE_URI,
    CMD_DENORMALIZE_URI,
    CMD_EXECUTE_BATCH,
    CMD_INSERT_EXT,
    CMD_BATCH_UPDATE,
};

enum class ISharedResultInterfaceCode {
    FUNC_GET_ROW_COUNT,
    FUNC_GET_ALL_COLUMN_NAMES,
    FUNC_ON_GO,
    FUNC_CLOSE,
    FUNC_GET_BLOB,
    FUNC_GET_STRING,
    FUNC_GET_INT,
    FUNC_GET_LONG,
    FUNC_GET_DOUBLE,
    FUNC_IS_COLUMN_NULL,
    FUNC_GO_TO,
    FUNC_GO_TO_ROW,
    FUNC_GO_TO_FISTR_ROW,
    FUNC_GO_TO_LAST_ROW,
    FUNC_GO_TO_NEXT_ROW,
    FUNC_GO_TO_PREV_ROW,
    FUNC_IS_AT_FIRST_ROW,
    FUNC_IS_AT_LAST_ROW,
    FUNC_IS_STARTED_ROW,
    FUNC_IS_ENDED_ROW,
    FUNC_IS_CLOSED,
    FUNC_GET_COLUMN_COUNT,
    FUNC_GET_COLUMN_INDEX,
    FUNC_GET_COLUMN_NAME,
    FUNC_GET_COLUMN_TYPE,
    FUNC_GET_ROW_INDEX,
    FUNC_BUTT,
};

enum class DataShareServiceInterfaceCode {
    DATA_SHARE_SERVICE_CMD_INSERT,
    DATA_SHARE_SERVICE_CMD_DELETE,
    DATA_SHARE_SERVICE_CMD_UPDATE,
    DATA_SHARE_SERVICE_CMD_QUERY,
    DATA_SHARE_SERVICE_CMD_ADD_TEMPLATE,
    DATA_SHARE_SERVICE_CMD_DEL_TEMPLATE,
    DATA_SHARE_SERVICE_CMD_PUBLISH,
    DATA_SHARE_SERVICE_CMD_GET_DATA,
    DATA_SHARE_SERVICE_CMD_SUBSCRIBE_RDB,
    DATA_SHARE_SERVICE_CMD_UNSUBSCRIBE_RDB,
    DATA_SHARE_SERVICE_CMD_ENABLE_SUBSCRIBE_RDB,
    DATA_SHARE_SERVICE_CMD_DISABLE_SUBSCRIBE_RDB,
    DATA_SHARE_SERVICE_CMD_SUBSCRIBE_PUBLISHED,
    DATA_SHARE_SERVICE_CMD_UNSUBSCRIBE_PUBLISHED,
    DATA_SHARE_SERVICE_CMD_ENABLE_SUBSCRIBE_PUBLISHED,
    DATA_SHARE_SERVICE_CMD_DISABLE_SUBSCRIBE_PUBLISHED,
    DATA_SHARE_SERVICE_CMD_NOTIFY,
    DATA_SHARE_SERVICE_CMD_NOTIFY_OBSERVERS,
    DATA_SHARE_SERVICE_CMD_SET_SILENT_SWITCH,
    DATA_SHARE_SERVICE_CMD_IS_SILENT_PROXY_ENABLE,
    DATA_SHARE_SERVICE_CMD_REGISTER_OBSERVER,
    DATA_SHARE_SERVICE_CMD_UNREGISTER_OBSERVER,
    DATA_SHARE_SERVICE_CMD_MAX
};

enum class IKvStoreDataInterfaceCode {
    GET_FEATURE_INTERFACE = 0,
    REGISTERCLIENTDEATHOBSERVER
};
}
} // namespace OHOS

#endif // DISTRIBUTEDDATA_DATA_SHARE_IPC_INTERFACE_H