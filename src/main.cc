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

#include <tclap/CmdLine.h>

#include "log.h"

// A description of the program.
constexpr char kDescription[] = "A tool for storing binary data on DAT tapes.";

// The version of the program.
constexpr char kVersion[] = "0.0.1";

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd(kDescription, ' ', kVersion);

  TCLAP::SwitchArg encode_mode_arg(
      "", "encode", "Put the tool in encode mode.", false /* req */);
  TCLAP::SwitchArg decode_mode_arg(
      "", "decode", "Put the tool in decode mode.", false /* req */);
  cmd.xorAdd(encode_mode_arg, decode_mode_arg);
  cmd.parse(argc, argv);

  int result = 0;
  if (encode_mode_arg.getValue()) {
    LOGE("Encode not implemented");
    result = -1;
  } else if (decode_mode_arg.getValue()) {
    LOGE("Decode not implemented");
    result = -1;
  }

  return result;
}
