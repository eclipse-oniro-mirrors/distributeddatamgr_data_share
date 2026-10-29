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
function anonymizeUri(uri) {
  if (uri == null || typeof uri !== 'string') {
    return String(uri);
  }
  const END_SIZE = 10;
  const DEFAULT_ANONYMOUS = '******';
  if (uri.length <= END_SIZE) {
    return DEFAULT_ANONYMOUS;
  }
  return DEFAULT_ANONYMOUS + uri.substring(uri.length - END_SIZE);
}

class DataShareExtensionAbility {
  onCreate(want, callback) {
    console.log('onCreate, want:' + want.abilityName);
    callback(undefined);
  }

  getFileTypes(uri, mimeTypeFilter, callback) {
    console.log('getFileTypes, uri:' + anonymizeUri(uri));
  }

  openFile(uri, mode, callback) {
    console.log('openFile, uri:' + anonymizeUri(uri));
  }

  openRawFile(uri, mode, callback) {
    console.log('openRawFile, uri:' + anonymizeUri(uri));
  }

  insert(uri, value, callback) {
    console.log('insert, uri:' + anonymizeUri(uri));
  }

  update(uri, predicates, value, callback) {
    console.log('update, uri:' + anonymizeUri(uri));
  }

  batchUpdate(operations, callback) {
    console.log('batchUpdate');
  }

  delete(uri, predicates, callback) {
    console.log('delete, uri:' + anonymizeUri(uri));
  }

  query(uri, predicates, columns, callback) {
    console.log('query, uri:' + anonymizeUri(uri));
  }

  getType(uri, callback) {
    console.log('getType, uri:' + anonymizeUri(uri));
  }

  batchInsert(uri, values, callback) {
    console.log('batchInsert, uri:' + anonymizeUri(uri));
  }

  normalizeUri(uri, callback) {
    console.log('normalizeUri, uri:' + anonymizeUri(uri));
  }

  denormalizeUri(uri, callback) {
    console.log('denormalizeUri, uri:' + anonymizeUri(uri));
  }
}

export default DataShareExtensionAbility;