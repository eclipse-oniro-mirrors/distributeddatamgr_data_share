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

#include "ishared_result_set_proxy.h"

#include "datashare_errno.h"
#include "datashare_log.h"
#include "iremote_proxy.h"
#include "string_ex.h"

using namespace OHOS::DistributedShare::DataShare;

namespace OHOS::DataShare {
std::function<std::shared_ptr<DataShareResultSet>(MessageParcel &parcel)> ISharedResultSet::consumerCreator_ =
    ISharedResultSetProxy::CreateProxy;
BrokerDelegator<ISharedResultSetProxy> ISharedResultSetProxy::delegator_;
ISharedResultSetProxy::ISharedResultSetProxy(const sptr<OHOS::IRemoteObject> &impl)
    : IRemoteProxy<ISharedResultSet>(impl)
{
}

std::shared_ptr<DataShareResultSet> ISharedResultSetProxy::CreateProxy(MessageParcel &parcel)
{
    sptr<IRemoteObject> remoter = parcel.ReadRemoteObject();
    if (remoter == nullptr) {
        return nullptr;
    }
    sptr<ISharedResultSetProxy> result = new (std::nothrow)ISharedResultSetProxy(remoter);
    if (result == nullptr) {
        return nullptr;
    }
    result->Unmarshalling(parcel);
    return std::shared_ptr<DataShareResultSet>(result.GetRefPtr(),
           [keep = result] (DataShareResultSet *) {});
}

int ISharedResultSetProxy::GetAllColumnNames(std::vector<std::string> &columnNames)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!columnNames_.empty()) {
        columnNames = columnNames_;
        return E_OK;
    }
    MessageParcel request;
    std::u16string descriptor = ISharedResultSetProxy::GetDescriptor();
    if (!request.WriteInterfaceToken(descriptor)) {
        LOG_ERROR("WriteDescriptor is failed, WriteDescriptor = %{public}s", Str16ToStr8(descriptor).c_str());
        return INVALID_FD;
    }
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(
        static_cast<uint32_t>(ISharedResultInterfaceCode::FUNC_GET_ALL_COLUMN_NAMES), request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("IPC Error %{public}x", errCode);
        return -errCode;
    }
    errCode = reply.ReadInt32();
    if (errCode != E_OK) {
        LOG_ERROR("Reply Error %{public}d", errCode);
        return errCode;
    }
    if (!reply.ReadStringVector(&columnNames)) {
        return E_INVALID_PARCEL;
    }
    columnNames_ = columnNames;
    return E_OK;
}

int ISharedResultSetProxy::GetRowCount(int &count)
{
    if (rowCount_ >= 0) {
        count = rowCount_;
        return E_OK;
    }
    MessageParcel request;
    std::u16string descriptor = ISharedResultSetProxy::GetDescriptor();
    if (!request.WriteInterfaceToken(descriptor)) {
        LOG_ERROR("WriteDescriptor is failed, WriteDescriptor = %{public}s", Str16ToStr8(descriptor).c_str());
        return INVALID_FD;
    }
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(
        static_cast<uint32_t>(ISharedResultInterfaceCode::FUNC_GET_ROW_COUNT), request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("IPC Error %{public}x", errCode);
        return -errCode;
    }
    errCode = reply.ReadInt32();
    if (errCode != E_OK) {
        LOG_ERROR("Reply Error %{public}d", errCode);
        return errCode;
    }
    count = reply.ReadInt32();
    LOG_DEBUG("count %{public}d", count);
    rowCount_ = count;
    return E_OK;
}

bool ISharedResultSetProxy::OnGo(int oldRowIndex, int newRowIndex, int *cachedIndex)
{
    MessageParcel request;
    std::u16string descriptor = ISharedResultSetProxy::GetDescriptor();
    if (!request.WriteInterfaceToken(descriptor)) {
        LOG_ERROR("WriteDescriptor is failed, WriteDescriptor = %{public}s", Str16ToStr8(descriptor).c_str());
        return false;
    }
    request.WriteInt32(oldRowIndex);
    request.WriteInt32(newRowIndex);
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(
        static_cast<uint32_t>(ISharedResultInterfaceCode::FUNC_ON_GO), request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("IPC Error %{public}x", errCode);
        return false;
    }
    int ret = reply.ReadInt32();
    if (cachedIndex != nullptr) {
        *cachedIndex = ret;
    }
    if (ret < 0) {
        return false;
    }
    return true;
}

int ISharedResultSetProxy::Close()
{
    DataShareResultSet::Close();
    MessageParcel request;
    std::u16string descriptor = ISharedResultSetProxy::GetDescriptor();
    if (!request.WriteInterfaceToken(descriptor)) {
        LOG_ERROR("WriteDescriptor is failed, WriteDescriptor = %{public}s", Str16ToStr8(descriptor).c_str());
        return INVALID_FD;
    }
    MessageParcel reply;
    MessageOption msgOption;
    int errCode = Remote()->SendRequest(
        static_cast<uint32_t>(ISharedResultInterfaceCode::FUNC_CLOSE), request, reply, msgOption);
    if (errCode != 0) {
        LOG_ERROR("IPC Error %{public}x", errCode);
        return -errCode;
    }
    return reply.ReadInt32();
}
} // namespace OHOS::DataShare