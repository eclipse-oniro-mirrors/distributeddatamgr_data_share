/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "published_data_subscriber_manager.h"

#include <cinttypes>

#include "data_proxy_observer_stub.h"
#include "datashare_log.h"
#include "datashare_string_utils.h"

namespace OHOS {
namespace DataShare {
std::vector<OperationResult> PublishedDataSubscriberManager::AddObservers(void *subscriber,
    std::shared_ptr<DataShareServiceProxy> proxy, const std::vector<std::string> &uris, int64_t subscriberId,
    const PublishedDataCallback &callback)
{
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr");
        return std::vector<OperationResult>();
    }
    std::vector<Key> keys;
    std::for_each(uris.begin(), uris.end(), [&keys, &subscriberId](auto &uri) {
        keys.emplace_back(uri, subscriberId);
    });
    return BaseCallbacks::AddObservers(
        keys, subscriber, std::make_shared<Observer>(callback),
        [this](const std::vector<Key> &localRegisterKeys, const std::shared_ptr<Observer> observer) {
            Emit(localRegisterKeys, observer);
        },
        [&proxy, subscriber, &subscriberId, this](const std::vector<Key> &firstAddKeys,
            const std::shared_ptr<Observer> observer, std::vector<OperationResult> &opResult) {
            std::vector<std::string> firstAddUris;
            std::for_each(firstAddKeys.begin(), firstAddKeys.end(), [&firstAddUris](auto &result) {
                firstAddUris.emplace_back(result);
            });
            if (firstAddUris.empty()) {
                return;
            }

            auto subResults = proxy->SubscribePublishedData(firstAddUris, subscriberId, serviceCallback_);
            std::vector<Key> failedKeys;
            for (auto &subResult : subResults) {
                opResult.emplace_back(subResult);
                if (subResult.errCode_ != E_OK) {
                    failedKeys.emplace_back(subResult.key_, subscriberId);
                    LOG_WARN("registered failed, uri is %{public}s", subResult.key_.c_str());
                }
            }
            if (failedKeys.size() > 0) {
                BaseCallbacks::DelObservers(failedKeys, subscriber);
            }
        });
}

std::vector<OperationResult> PublishedDataSubscriberManager::DelObservers(void *subscriber,
    std::shared_ptr<DataShareServiceProxy> proxy)
{
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr");
        return std::vector<OperationResult>();
    }
    return BaseCallbacks::DelObservers(subscriber,
        [&proxy, this](const std::vector<Key> &lastDelKeys, std::vector<OperationResult> &opResult) {
            // delete all obs by subscriber
            std::map<int64_t, std::vector<std::string>> keysMap;
            for (auto const &key : lastDelKeys) {
                lastChangeNodeMap_.Erase(key);
                keysMap[key.subscriberId_].emplace_back(key.uri_);
            }
            for (auto const &[subscriberId, uris] : keysMap) {
                auto results = proxy->UnSubscribePublishedData(uris, subscriberId);
                opResult.insert(opResult.end(), results.begin(), results.end());
            }
        });
}

std::vector<OperationResult> PublishedDataSubscriberManager::DelObservers(void *subscriber,
    std::shared_ptr<DataShareServiceProxy> proxy, const std::vector<std::string> &uris, int64_t subscriberId)
{
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr");
        return std::vector<OperationResult>();
    }
    if (uris.empty()) {
        return DelObservers(subscriber, proxy);
    }

    std::vector<Key> keys;
    std::for_each(uris.begin(), uris.end(), [&keys, &subscriberId](auto &uri) {
        keys.emplace_back(uri, subscriberId);
    });
    return BaseCallbacks::DelObservers(keys, subscriber,
        [&proxy, &subscriberId, this](const std::vector<Key> &lastDelKeys, std::vector<OperationResult> &opResult) {
            std::vector<std::string> lastDelUris;
            std::for_each(lastDelKeys.begin(), lastDelKeys.end(), [&lastDelUris, this](auto &result) {
                lastChangeNodeMap_.Erase(result);
                lastDelUris.emplace_back(result);
            });
            if (lastDelUris.empty()) {
                return;
            }
            auto unsubResult = proxy->UnSubscribePublishedData(lastDelUris, subscriberId);
            opResult.insert(opResult.end(), unsubResult.begin(), unsubResult.end());
        });
}

std::vector<OperationResult> PublishedDataSubscriberManager::EnableObservers(void *subscriber,
    std::shared_ptr<DataShareServiceProxy> proxy, const std::vector<std::string> &uris, int64_t subscriberId)
{
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr");
        return std::vector<OperationResult>();
    }
    std::vector<Key> keys;
    std::for_each(uris.begin(), uris.end(), [&keys, &subscriberId](auto &uri) {
        keys.emplace_back(uri, subscriberId);
    });
    return BaseCallbacks::EnableObservers(
        keys, subscriber, [this](std::map<Key, std::vector<ObserverNodeOnEnabled>> &obsMap) {
            EmitOnEnable(obsMap);
        },
        [&proxy, &subscriberId, subscriber, this](const std::vector<Key> &firstAddKeys,
        std::vector<OperationResult> &opResult) {
            std::vector<std::string> firstAddUris;
            std::for_each(firstAddKeys.begin(), firstAddKeys.end(), [&firstAddUris](auto &result) {
                firstAddUris.emplace_back(result);
            });
            if (firstAddUris.empty()) {
                return;
            }
            auto subResults = proxy->EnableSubscribePublishedData(firstAddUris, subscriberId);
            std::vector<Key> failedKeys;
            for (auto &subResult : subResults) {
                opResult.emplace_back(subResult);
                if (subResult.errCode_ != E_OK) {
                    failedKeys.emplace_back(subResult.key_, subscriberId);
                    LOG_WARN("registered failed, uri is %{public}s", subResult.key_.c_str());
                }
            }
            if (failedKeys.size() > 0) {
                BaseCallbacks::DisableObservers(failedKeys, subscriber);
            }
        });
}

std::vector<OperationResult> PublishedDataSubscriberManager::DisableObservers(void *subscriber,
    std::shared_ptr<DataShareServiceProxy> proxy, const std::vector<std::string> &uris, int64_t subscriberId)
{
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr");
        return std::vector<OperationResult>();
    }
    std::vector<Key> keys;
    std::for_each(uris.begin(), uris.end(), [&keys, &subscriberId](auto &uri) {
        keys.emplace_back(uri, subscriberId);
    });
    return BaseCallbacks::DisableObservers(keys, subscriber,
        [&proxy, &subscriberId, subscriber, this](const std::vector<Key> &lastDisabledKeys,
        std::vector<OperationResult> &opResult) {
            std::vector<std::string> lastDisabledUris;
            std::for_each(lastDisabledKeys.begin(), lastDisabledKeys.end(), [&lastDisabledUris](auto &result) {
                lastDisabledUris.emplace_back(result);
            });
            if (lastDisabledUris.empty()) {
                return;
            }

            auto results = proxy->DisableSubscribePublishedData(lastDisabledUris, subscriberId);
            std::vector<Key> failedKeys;
            for (auto &result : results) {
                opResult.emplace_back(result);
                if (result.errCode_ != E_OK) {
                    failedKeys.emplace_back(result.key_, subscriberId);
                    LOG_WARN("DisableObservers failed, uri is %{public}s, errCode is %{public}d", result.key_.c_str(),
                        result.errCode_);
                }
            }
            if (failedKeys.size() > 0) {
                BaseCallbacks::EnableObservers(failedKeys, subscriber);
            }
        });
}

void PublishedDataSubscriberManager::RecoverObservers(std::shared_ptr<DataShareServiceProxy> proxy)
{
    if (proxy == nullptr) {
        LOG_ERROR("proxy is nullptr");
        return;
    }

    std::map<int64_t, std::vector<std::string>> keysMap;
    std::vector<Key> keys = CallbacksManager::GetKeys();
    for (const auto& key : keys) {
        keysMap[key.subscriberId_].emplace_back(key.uri_);
    }
    for (const auto &[subscriberId, uris] : keysMap) {
        auto results = proxy->SubscribePublishedData(uris, subscriberId, serviceCallback_);
        for (const auto& result : results) {
            if (result.errCode_ != E_OK) {
                LOG_WARN("RecoverObservers failed, uri is %{public}s, errCode is %{public}d",
                         result.key_.c_str(), result.errCode_);
            }
        }
    }
}

void PublishedDataSubscriberManager::Emit(PublishedDataChangeNode &changeNode)
{
    std::map<std::shared_ptr<Observer>, PublishedDataChangeNode> results;
    for (auto &data : changeNode.datas_) {
        PublishedObserverMapKey key(data.key_, data.subscriberId_);
        // Still set observer was notified flag and store data if there is no enabled observer.
        BaseCallbacks::SetObserversNotifiedOnEnabled(key);
        auto callbacks = BaseCallbacks::GetEnabledObservers(key);
        lastChangeNodeMap_.Compute(key, [&data, &changeNode](const Key &, PublishedDataChangeNode &value) {
            value.datas_.clear();
            value.datas_.emplace_back(data.key_, data.subscriberId_, data.GetData());
            value.ownerBundleName_ = changeNode.ownerBundleName_;
            return true;
        });

        if (callbacks.empty()) {
            LOG_WARN("%{private}s nobody subscribe, but still notify", data.key_.c_str());
            continue;
        }
        for (auto const &obs : callbacks) {
            results[obs].datas_.emplace_back(data.key_, data.subscriberId_, data.GetData());
        }
    }
    for (auto &[callback, node] : results) {
        node.ownerBundleName_ = changeNode.ownerBundleName_;
        callback->OnChange(node);
    }
}

void PublishedDataSubscriberManager::Emit(const std::vector<Key> &keys, const std::shared_ptr<Observer> &observer)
{
    PublishedDataChangeNode node;
    for (auto &key : keys) {
        lastChangeNodeMap_.ComputeIfPresent(key, [&node](const Key &, PublishedDataChangeNode &value) {
            for (auto &data : value.datas_) {
                node.datas_.emplace_back(data.key_, data.subscriberId_, data.GetData());
            }
            node.ownerBundleName_ = value.ownerBundleName_;
            return true;
        });
    }
    observer->OnChange(node);
}

void PublishedDataSubscriberManager::EmitOnEnable(std::map<Key, std::vector<ObserverNodeOnEnabled>> &obsMap)
{
    std::map<std::shared_ptr<Observer>, PublishedDataChangeNode> results;
    for (auto &[key, obsVector] : obsMap) {
        uint32_t num = 0;
        lastChangeNodeMap_.ComputeIfPresent(key, [&obsVector = obsVector, &results, &num](const Key &,
            PublishedDataChangeNode &value) {
            for (auto &data : value.datas_) {
                PublishedObserverMapKey mapKey(data.key_, data.subscriberId_);
                for (auto &obs : obsVector) {
                    if (obs.isNotifyOnEnabled_) {
                        num++;
                        results[obs.observer_].datas_.emplace_back(data.key_, data.subscriberId_, data.GetData());
                        results[obs.observer_].ownerBundleName_ = value.ownerBundleName_;
                        obs.isNotifyOnEnabled_ = false;
                    }
                }
            }
            return true;
        });
        if (num > 0) {
            LOG_INFO("%{public}u will refresh, total %{public}zu, uri %{public}s, subscribeId %{public}" PRId64,
                num, obsVector.size(), DataShareStringUtils::Anonymous(key.uri_).c_str(), key.subscriberId_);
        }
    }
    for (auto &[callback, node] : results) {
        callback->OnChange(node);
    }
}

PublishedDataSubscriberManager &PublishedDataSubscriberManager::GetInstance()
{
    static PublishedDataSubscriberManager manager;
    return manager;
}

PublishedDataSubscriberManager::PublishedDataSubscriberManager()
{
    serviceCallback_ = new (std::nothrow)PublishedDataObserverStub([this](PublishedDataChangeNode &changeNode) {
        Emit(changeNode);
    });
}

PublishedDataObserver::PublishedDataObserver(const PublishedDataCallback &callback) : callback_(callback) {}

void PublishedDataObserver::OnChange(PublishedDataChangeNode &changeNode)
{
    callback_(changeNode);
}

bool PublishedDataObserver::operator==(const PublishedDataObserver &rhs) const
{
    return false;
}

bool PublishedDataObserver::operator!=(const PublishedDataObserver &rhs) const
{
    return !(rhs == *this);
}
} // namespace DataShare
} // namespace OHOS
