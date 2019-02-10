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

#include "datvideo/crc16.h"
#include "datvideo/log.h"

namespace {

// A description of the program.
constexpr char kDescription[] = "A tool for storing binary data on DAT tapes.";

// The version of the program.
constexpr char kVersion[] = "0.0.1";

// The default size of an MPEG-TS frame with no error correction.
constexpr size_t kMpegTsFrameSize = 188;

}  // namespace 

namespace datvideo {

//! The delimiter byte to separate frames.
constexpr uint8_t kRfc1662Delimiter = 0x7e;

//! The escape byte to escape delimiters and escapes themselves.
constexpr uint8_t kRfc1662Escape = 0x7d;

/**
 * Inserts the supplied byte into the frame, escaping if necessary.
 */
void InsertRfc1662EscapedByte(uint8_t byte, std::vector<uint8_t>* frame) {
  if (byte == kRfc1662Delimiter || byte == kRfc1662Escape) {
    frame->push_back(kRfc1662Escape);
  }

  frame->push_back(byte);
}

/**
 * Encodes the supplied chunk into an RFC1662 frame.
 */
void EncodeRfc1662Frame(const std::vector<uint8_t>& chunk,
                        std::vector<uint8_t>* frame) {
  frame->clear();
  frame->push_back(kRfc1662Delimiter);
  for (uint8_t byte : chunk) {
    InsertRfc1662EscapedByte(byte, frame);
  }

  uint16_t crc = datvideo::GenerateCrc16(chunk.data(), chunk.size());
  InsertRfc1662EscapedByte(crc >> 8, frame);
  InsertRfc1662EscapedByte(crc, frame);
  frame->push_back(kRfc1662Delimiter);
}

/**
 * Encodes the supplied input file into the supplied output file as RFC-1662
 * frames containing chunk_size worth of file each.
 */
bool EncodeFile(std::FILE* in_file, std::FILE* out_file, size_t chunk_size) {
  size_t bytes_read = 0;
  std::vector<uint8_t> buf(chunk_size);
  std::vector<uint8_t> frame;
  while ((bytes_read = std::fread(buf.data(), sizeof(buf[0]),
                                  chunk_size, in_file)) > 0) {
    buf.resize(bytes_read);
    EncodeRfc1662Frame(buf, &frame);
    size_t bytes_written = std::fwrite(frame.data(), sizeof(frame[0]),
                                       frame.size(), out_file);
    if (bytes_written != frame.size()) {
      LOGE("Failed to write frame: %d", ferror(out_file));
    }
  }

  return true;
}

/**
 * Decodes the supplied input file into the supplied output file, assuming
 * RFC-1662 frames as the input.
 */
bool DecodeFile(std::FILE* in_file, std::FILE* out_file) {
  // The maximum size for a given frame. This is pretty huge.
  constexpr size_t kMaxFrameSize = 1024 * 1024;

  enum class ReceiveState {
    Reset,
    InFrame,
    InEscape,
  };

  uint8_t byte;
  size_t bytes_read = 0;
  std::vector<uint8_t> frame;
  ReceiveState receive_state = ReceiveState::Reset;
  while ((bytes_read = std::fread(&byte, sizeof(byte), 1, in_file)) > 0) {
    if (receive_state == ReceiveState::Reset) {
      if (byte == kRfc1662Delimiter) {
        receive_state = ReceiveState::InFrame;
      }
    } else if (receive_state == ReceiveState::InFrame) {
      if (byte == kRfc1662Delimiter) {
        if (frame.size() >= sizeof(uint16_t)) {
          uint16_t crc = ((static_cast<uint16_t>(frame[frame.size() - 2]) << 8)
              | (frame[frame.size() - 1]));
          uint16_t computed_crc =
              GenerateCrc16(frame.data(), frame.size() - sizeof(uint16_t));
          if (crc == computed_crc) {
            size_t frame_size = frame.size() - sizeof(uint16_t);
            size_t bytes_written = std::fwrite(
                frame.data(), sizeof(frame[0]), frame_size, out_file);
            if (bytes_written != frame_size) {
              LOGE("Failed to write frame: %d", ferror(out_file));
            }
          } else {
            LOGW("CRC mismatch received");
          }
        } else {
          LOGW("Short frame received");
        }

        frame.clear();
        receive_state = ReceiveState::Reset;
      } else if (byte == kRfc1662Escape) {
        receive_state = ReceiveState::InEscape;
      } else if (frame.size() > kMaxFrameSize) {
        LOGW("Long frame received");
        frame.clear();
        receive_state = ReceiveState::Reset;
      } else {
        frame.push_back(byte);
      }
    } else if (receive_state == ReceiveState::InEscape) {
      if (byte == kRfc1662Delimiter || byte == kRfc1662Escape) {
        frame.push_back(byte);
        receive_state = ReceiveState::InFrame;
      } else {
        LOGW("Invalid escape sequence received");
        frame.clear();
        receive_state = ReceiveState::Reset;
      }
    }
  }

  return true;
}

}  // namespace datvideo

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
    result = datvideo::EncodeFile(in_file, out_file, chunk_size_arg.getValue());
  } else if (decode_mode_arg.getValue()) {
    result = datvideo::DecodeFile(in_file, out_file);
  }

  return (result ? 0 : -1);
}
