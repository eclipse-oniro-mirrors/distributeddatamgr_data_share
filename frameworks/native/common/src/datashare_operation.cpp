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

#include "datashare_operation.h"
#include "datashare_log.h"

namespace OHOS {
namespace DataShare {
DataShareOperation::DataShareOperation(
    const std::shared_ptr<DataShareOperation> &dataShareOperation, const std::shared_ptr<Uri> &withUri)
{
    uri_ = withUri;
    if (dataShareOperation != nullptr) {
        type_ = dataShareOperation->type_;
        valuesBucket_ = dataShareOperation->valuesBucket_;
        expectedCount_ = dataShareOperation->expectedCount_;
        dataSharePredicates_ = dataShareOperation->dataSharePredicates_;
        valuesBucketReferences_ = dataShareOperation->valuesBucketReferences_;
        dataSharePredicatesBackReferences_ = dataShareOperation->dataSharePredicatesBackReferences_;
        interrupted_ = dataShareOperation->interrupted_;
    } else {
        type_ = 0;
        expectedCount_ = 0;
        valuesBucket_ = std::make_shared<DataShareValuesBucket>();
        dataSharePredicates_ = std::make_shared<DataSharePredicates>();
        valuesBucketReferences_ = std::make_shared<DataShareValuesBucket>();
        dataSharePredicatesBackReferences_.clear();
        interrupted_ = false;
    }
}
DataShareOperation::DataShareOperation(Parcel &in)
{
    ReadFromParcel(in);
}
DataShareOperation::DataShareOperation(const std::shared_ptr<DataShareOperationBuilder> &builder)
{
    if (builder != nullptr) {
        type_ = builder->type_;
        uri_ = builder->uri_;
        valuesBucket_ = builder->valuesBucket_;
        expectedCount_ = builder->expectedCount_;
        dataSharePredicates_ = builder->dataSharePredicates_;
        valuesBucketReferences_ = builder->valuesBucketReferences_;
        dataSharePredicatesBackReferences_ = builder->dataSharePredicatesBackReferences_;
        interrupted_ = builder->interrupted_;
    }
}

DataShareOperation::DataShareOperation()
{
    type_ = 0;
    uri_ = nullptr;
    expectedCount_ = 0;
    valuesBucket_ = std::make_shared<DataShareValuesBucket>();
    dataSharePredicates_ = std::make_shared<DataSharePredicates>();
    valuesBucketReferences_ = std::make_shared<DataShareValuesBucket>();
    dataSharePredicatesBackReferences_.clear();
    interrupted_ = false;
}

DataShareOperation::~DataShareOperation()
{
    dataSharePredicatesBackReferences_.clear();
}

bool DataShareOperation::operator==(const DataShareOperation &other) const
{
    if (type_ != other.type_) {
        return false;
    }
    if ((uri_ != nullptr) && (other.uri_ != nullptr) && (uri_->ToString() != other.uri_->ToString())) {
        return false;
    }
    if (expectedCount_ != other.expectedCount_) {
        return false;
    }
    if (valuesBucket_ != other.valuesBucket_) {
        return false;
    }
    if (dataSharePredicates_ != other.dataSharePredicates_) {
        return false;
    }
    if (valuesBucketReferences_ != other.valuesBucketReferences_) {
        return false;
    }
    size_t backReferencesCount = dataSharePredicatesBackReferences_.size();
    size_t otherBackReferencesCount = other.dataSharePredicatesBackReferences_.size();
    if (backReferencesCount != otherBackReferencesCount) {
        return false;
    }

    std::map<int, int>::const_iterator it = dataSharePredicatesBackReferences_.begin();
    while (it != dataSharePredicatesBackReferences_.end()) {
        std::map<int, int>::const_iterator otherIt = other.dataSharePredicatesBackReferences_.find(it->first);
        if (otherIt != other.dataSharePredicatesBackReferences_.end()) {
            if (otherIt->second != it->second) {
                return false;
            }
        } else {
            return false;
        }
        ++it;
    }

    if (interrupted_ != other.interrupted_) {
        return false;
    }
    return true;
}

DataShareOperation &DataShareOperation::operator=(const DataShareOperation &other)
{
    if (this != &other) {
        type_ = other.type_;
        uri_ = other.uri_;
        expectedCount_ = other.expectedCount_;
        valuesBucket_ = other.valuesBucket_;
        dataSharePredicates_ = other.dataSharePredicates_;
        valuesBucketReferences_ = other.valuesBucketReferences_;
        dataSharePredicatesBackReferences_ = other.dataSharePredicatesBackReferences_;
        interrupted_ = other.interrupted_;
    }
    return *this;
}

std::shared_ptr<DataShareOperationBuilder> DataShareOperation::NewInsertBuilder(const std::shared_ptr<Uri> &uri)
{
    LOG_DEBUG("DataShareOperation::NewInsertBuilder start");
    if (uri == nullptr) {
        LOG_ERROR("DataShareOperation::NewInsertBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataShareOperationBuilder> builder =
        std::make_shared<DataShareOperationBuilder>(TYPE_INSERT, uri);
    LOG_DEBUG("DataShareOperation::NewInsertBuilder end");
    return builder;
}

std::shared_ptr<DataShareOperationBuilder> DataShareOperation::NewUpdateBuilder(const std::shared_ptr<Uri> &uri)
{
    LOG_DEBUG("DataShareOperation::NewUpdateBuilder start");
    if (uri == nullptr) {
        LOG_ERROR("DataShareOperation::NewUpdateBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataShareOperationBuilder> builder =
        std::make_shared<DataShareOperationBuilder>(TYPE_UPDATE, uri);
    LOG_DEBUG("DataShareOperation::NewUpdateBuilder end");
    return builder;
}

std::shared_ptr<DataShareOperationBuilder> DataShareOperation::NewDeleteBuilder(const std::shared_ptr<Uri> &uri)
{
    LOG_DEBUG("DataShareOperation::NewDeleteBuilder start");
    if (uri == nullptr) {
        LOG_ERROR("DataShareOperation::NewDeleteBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataShareOperationBuilder> builder =
        std::make_shared<DataShareOperationBuilder>(TYPE_DELETE, uri);
    LOG_DEBUG("DataShareOperation::NewDeleteBuilder end");
    return builder;
}

std::shared_ptr<DataShareOperationBuilder> DataShareOperation::NewAssertBuilder(const std::shared_ptr<Uri> &uri)
{
    LOG_DEBUG("DataShareOperation::NewAssertBuilder start");
    if (uri == nullptr) {
        LOG_ERROR("DataShareOperation::NewAssertBuilder uri is nullptr");
        return nullptr;
    }
    std::shared_ptr<DataShareOperationBuilder> builder =
        std::make_shared<DataShareOperationBuilder>(TYPE_ASSERT, uri);
    LOG_DEBUG("DataShareOperation::NewAssertBuilder end");
    return builder;
}

int DataShareOperation::GetType() const
{
    LOG_DEBUG("DataShareOperation::GetType");
    return type_;
}

std::shared_ptr<Uri> DataShareOperation::GetUri() const
{
    LOG_DEBUG("DataShareOperation::GetUri");
    return uri_;
}

std::shared_ptr<DataShareValuesBucket> DataShareOperation::GetValuesBucket() const
{
    LOG_DEBUG("DataShareOperation::GetValuesBucket");
    return valuesBucket_;
}

int DataShareOperation::GetExpectedCount() const
{
    LOG_DEBUG("DataShareOperation::GetExpectedCount");
    return expectedCount_;
}

std::shared_ptr<DataSharePredicates> DataShareOperation::GetDataSharePredicates() const
{
    LOG_DEBUG("DataShareOperation::GetDataSharePredicates");
    return dataSharePredicates_;
}

std::shared_ptr<DataShareValuesBucket> DataShareOperation::GetValuesBucketReferences() const
{
    LOG_DEBUG("DataShareOperation::GetValuesBucketReferences");
    return valuesBucketReferences_;
}
std::map<int, int> DataShareOperation::GetDataSharePredicatesBackReferences() const
{
    LOG_DEBUG("DataShareOperation::GetDataSharePredicatesBackReferences");
    return dataSharePredicatesBackReferences_;
}
bool DataShareOperation::IsInsertOperation() const
{
    LOG_DEBUG("DataShareOperation::IsInsertOperation：%d", type_ == TYPE_INSERT);
    return type_ == TYPE_INSERT;
}
bool DataShareOperation::IsUpdateOperation() const
{
    LOG_DEBUG("DataShareOperation::IsUpdateOperation：%d", type_ == TYPE_UPDATE);
    return type_ == TYPE_UPDATE;
}
bool DataShareOperation::IsDeleteOperation() const
{
    LOG_DEBUG("DataShareOperation::IsDeleteOperation：%d", type_ == TYPE_DELETE);
    return type_ == TYPE_DELETE;
}
bool DataShareOperation::IsAssertOperation() const
{
    LOG_DEBUG("DataShareOperation::IsAssertOperation：%d", type_ == TYPE_ASSERT);
    return type_ == TYPE_ASSERT;
}
bool DataShareOperation::IsInterruptionAllowed() const
{
    LOG_DEBUG("DataShareOperation::IsInterruptionAllowed：%d", interrupted_);
    return interrupted_;
}
bool DataShareOperation::Marshalling(Parcel &out) const
{
    LOG_DEBUG("DataShareOperation::Marshalling start");
    if (!out.WriteInt32(type_)) {
        LOG_ERROR("DataShareOperation::Marshalling WriteInt32(type_) error");
        return false;
    }
    if (!out.WriteInt32(expectedCount_)) {
        LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
        return false;
    }

    if (!out.WriteBool(interrupted_)) {
        LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
        return false;
    }

    if (uri_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }

        if (!out.WriteParcelable(uri_.get())) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            return false;
        }
    }

    if (valuesBucket_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }

        if (!DataShareValuesBucket::Marshalling(*valuesBucket_, out)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    }

    if (dataSharePredicates_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (!out.WriteParcelable(dataSharePredicates_.get())) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            return false;
        }
    }

    if (valuesBucketReferences_ != nullptr) {
        if (!out.WriteInt32(VALUE_OBJECT)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (!DataShareValuesBucket::Marshalling(*valuesBucketReferences_, out)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    } else {
        if (!out.WriteInt32(VALUE_NULL)) {
            return false;
        }
    }

    int referenceSize = 0;
    if (!dataSharePredicatesBackReferences_.empty()) {
        referenceSize = (int)dataSharePredicatesBackReferences_.size();
        if (!out.WriteInt32(referenceSize)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
        if (referenceSize >= REFERENCE_THRESHOLD) {
            LOG_INFO("DataShareOperation::Marshalling referenceSize >= REFERENCE_THRESHOLD");
            return true;
        }
        for (auto &it : dataSharePredicatesBackReferences_) {
            if (!out.WriteInt32(it.first)) {
                LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
                return false;
            }
            if (!out.WriteInt32(it.second)) {
                LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
                return false;
            }
        }
    } else {
        LOG_DEBUG("DataShareOperation::Marshalling dataSharePredicatesBackReferences_ is empty");
        if (!out.WriteInt32(referenceSize)) {
            LOG_ERROR("DataShareOperation::Marshalling WriteInt32(VALUE_OBJECT) error");
            return false;
        }
    }

    LOG_DEBUG("DataShareOperation::Marshalling end");
    return true;
}
DataShareOperation *DataShareOperation::Unmarshalling(Parcel &in)
{
    LOG_DEBUG("DataShareOperation::Unmarshalling start");
    DataShareOperation *dataShareOperation = new (std::nothrow) DataShareOperation();
    if (dataShareOperation != nullptr && !dataShareOperation->ReadFromParcel(in)) {
        LOG_ERROR("DataShareOperation::Unmarshalling dataShareOperation error");
        delete dataShareOperation;
        dataShareOperation = nullptr;
    }
    LOG_DEBUG("DataShareOperation::Unmarshalling end");
    return dataShareOperation;
}
bool DataShareOperation::ReadFromParcel(Parcel &in)
{
    LOG_DEBUG("DataShareOperation::ReadFromParcel start");
    if (!in.ReadInt32(type_)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel ReadInt32(type_) error");
        return false;
    }
    if (!in.ReadInt32(expectedCount_)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    interrupted_ = in.ReadBool();
    int empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    if (empty == VALUE_OBJECT) {
        uri_.reset(in.ReadParcelable<Uri>());
    } else {
        uri_.reset();
    }
    empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    LOG_DEBUG("DataShareOperation::ReadFromParcel empty is %{public}s",
        empty == VALUE_OBJECT ? "VALUE_OBJECT" : "VALUE_NULL");
    if (empty == VALUE_OBJECT) {
        valuesBucket_.reset(DataShareValuesBucket::Unmarshalling(in));
    } else {
        valuesBucket_.reset();
    }
    empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    LOG_DEBUG("DataShareOperation::ReadFromParcel empty is %{public}s",
        empty == VALUE_OBJECT ? "VALUE_OBJECT" : "VALUE_NULL");
    if (empty == VALUE_OBJECT) {
        dataSharePredicates_.reset(in.ReadParcelable<DataSharePredicates>());
    } else {
        dataSharePredicates_.reset();
    }
    empty = VALUE_NULL;
    if (!in.ReadInt32(empty)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel ReadInt32(empty) error");
        return false;
    }
    LOG_DEBUG("DataShareOperation::ReadFromParcel empty is %{public}s",
        (empty == VALUE_OBJECT) ? "VALUE_OBJECT" : "VALUE_NULL");
    if (empty == VALUE_OBJECT) {
        valuesBucketReferences_.reset(DataShareValuesBucket::Unmarshalling(in));
    } else {
        valuesBucketReferences_.reset();
    }
    int referenceSize = 0;
    if (!in.ReadInt32(referenceSize)) {
        LOG_ERROR("DataShareOperation::ReadFromParcel end");
        return false;
    }
    if (referenceSize >= REFERENCE_THRESHOLD) {
        LOG_INFO("DataShareOperation::ReadFromParcel referenceSize:%{public}d >= REFERENCE_THRESHOLD:%{public}d",
            referenceSize, REFERENCE_THRESHOLD);
        return true;
    }
    for (int i = 0; i < REFERENCE_THRESHOLD && i < referenceSize; ++i) {
        int first = 0;
        int second = 0;
        if (!in.ReadInt32(first)) {
            LOG_ERROR("DataShareOperation::ReadFromParcel end");
            return false;
        }
        if (!in.ReadInt32(second)) {
            LOG_ERROR("DataShareOperation::ReadFromParcel end");
            return false;
        }
        dataSharePredicatesBackReferences_.insert(std::make_pair(first, second));
    }
    LOG_DEBUG("DataShareOperation::ReadFromParcel end");
    return true;
}
std::shared_ptr<DataShareOperation> DataShareOperation::CreateFromParcel(Parcel &in)
{
    LOG_DEBUG("DataShareOperation::CreateFromParcel start");
    std::shared_ptr<DataShareOperation> operation = std::make_shared<DataShareOperation>(in);
    LOG_DEBUG("DataShareOperation::CreateFromParcel end");
    return operation;
}
void DataShareOperation::PutMap(Parcel &in)
{
    LOG_DEBUG("DataShareOperation::PutMap start");
    int count = in.ReadInt32();
    if (count > 0 && count < REFERENCE_THRESHOLD) {
        for (int i = 0; i < count; ++i) {
            dataSharePredicatesBackReferences_.insert(std::make_pair(in.ReadInt32(), in.ReadInt32()));
        }
    }
    LOG_DEBUG("DataShareOperation::PutMap end");
}
}  // namespace DataShare
}  // namespace OHOS