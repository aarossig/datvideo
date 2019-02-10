/*
 * Copyright 2019 Andrew Rossignol (andrew.rossignol@gmail.com)
 * 
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

#ifndef DATVIDEO_CRC16_H_
#define DATVIDEO_CRC16_H_

#include <cstddef>
#include <cstdint>

namespace datvideo {

uint16_t GenerateCrc16(const uint8_t *buffer, size_t length);

}  // namespace datvideo

#endif  // DATVIDEO_CRC16_H_
