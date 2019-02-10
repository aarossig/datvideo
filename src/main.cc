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

#include <fstream>
#include <string>
#include <vector>

#include <tclap/CmdLine.h>

#include "log.h"

// A description of the program.
constexpr char kDescription[] = "A tool for storing binary data on DAT tapes.";

// The version of the program.
constexpr char kVersion[] = "0.0.1";

// The default size of an MPEG-TS frame with no error correction.
constexpr size_t kMpegTsFrameSize = 188;

/**
 * Encodes the supplied input file into the supplied output file as RFC-1662
 * frames containing chunk_size worth of file each.
 */
bool EncodeFile(std::FILE* in_file, std::FILE* out_file, size_t chunk_size) {
  size_t bytes_read = 0;
  std::vector<char> buf(chunk_size);
  while ((bytes_read = std::fread(buf.data(), sizeof(buf[0]),
                                  chunk_size, in_file)) > 0) {

    // TODO: Encode into RFC-1662 frames.

    size_t bytes_written = std::fwrite(buf.data(), sizeof(buf[0]),
                                       bytes_read, out_file);
    if (bytes_written != bytes_read /* TODO: frame_size */) {
      LOGE("Failed to write frame: %d", ferror(out_file));
    }
  }

  return true;
}

int main(int argc, char **argv) {
  TCLAP::CmdLine cmd(kDescription, ' ', kVersion);

  TCLAP::SwitchArg encode_mode_arg(
      "e", "encode", "Put the tool in encode mode.", false /* req */);
  TCLAP::SwitchArg decode_mode_arg(
      "d", "decode", "Put the tool in decode mode.", false /* req */);
  cmd.xorAdd(encode_mode_arg, decode_mode_arg);

  TCLAP::ValueArg<std::string> in_file_arg(
      "i", "input_file", "The input file to use for the current operation. "
      "Do not specify for stdin.", false /* req */, "", "path", cmd);
  TCLAP::ValueArg<std::string> out_file_arg(
      "o", "output_file", "The output file to use for the current operation. "
      "Do not specify for stdout.", false /* req */, "", "path", cmd);
  TCLAP::ValueArg<size_t> chunk_size_arg(
      "s", "chunk_size", "The size of chunks to split the file into. "
      "This is useful for streaming operations, like audio/video.",
      false /* req */, kMpegTsFrameSize, "byte count", cmd);
  cmd.parse(argc, argv);

  std::FILE *in_file = stdin;
  if (!in_file_arg.getValue().empty()) {
    in_file = fopen(in_file_arg.getValue().c_str(), "rb");
  } else {
    std::freopen(nullptr, "rb", stdin);
  }

  std::FILE *out_file = stdout;
  if (!out_file_arg.getValue().empty()) {
    out_file = fopen(out_file_arg.getValue().c_str(), "wb");
  } else {
    std::freopen(nullptr, "wb", stdout);
  }

  bool result = false;
  if (!in_file) {
    LOGE("Failed to open input file");
  } else if (!out_file) {
    LOGE("Failed to open output file");
  } else if (encode_mode_arg.getValue()) {
    result = EncodeFile(in_file, out_file, chunk_size_arg.getValue());
  } else if (decode_mode_arg.getValue()) {
    LOGE("Decode not implemented");
  }

  return (result ? 0 : -1);
}
