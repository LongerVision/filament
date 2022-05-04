/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FNodeManager.h"

#include "upcast.h"

using namespace utils;

namespace gltfio {

using Instance = NodeManager::Instance;

bool NodeManager::hasComponent(Entity e) const noexcept {
    return upcast(this)->hasComponent(e);
}

Instance NodeManager::getInstance(Entity e) const noexcept {
    return upcast(this)->getInstance(e);
}

void NodeManager::create(Entity entity) {
    upcast(this)->create(entity);
}

void NodeManager::destroy(Entity e) noexcept {
    upcast(this)->destroy(e);
}

void NodeManager::setMorphTargetNames(Instance ci, FixedCapacityVector<CString>* names) noexcept {
    upcast(this)->setMorphTargetNames(ci, names);
}

const FixedCapacityVector<CString>* NodeManager::getMorphTargetNames(Instance ci) const noexcept {
    return upcast(this)->getMorphTargetNames(ci);
}

void NodeManager::setExtras(Instance ci, CString&& extras) noexcept {
    return upcast(this)->setExtras(ci, std::move(extras));
}

const NodeManager::CString& NodeManager::getExtras(Instance ci) const noexcept {
    return upcast(this)->getExtras(ci);
}

void NodeManager::setSceneMembership(Instance ci, bitset32 scenes) noexcept {
    upcast(this)->setSceneMembership(ci, scenes);
}

bitset32 NodeManager::getSceneMembership(Instance ci) const noexcept {
    return upcast(this)->getSceneMembership(ci);
}

} // namespace gltfio
