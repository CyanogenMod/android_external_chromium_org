/*-
 * Copyright 2003,2004 Colin Percival
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Changelog:
 * 2009-03-31 - Change to use Streams.  Move CRC code to crc.{h,cc}
 *                --Stephen Adams <sra@chromium.org>
 */

// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/courgette/bsdiff.h"

#include "third_party/courgette/crc.h"
#include "third_party/courgette/streams.h"

namespace courgette {

BSDiffStatus MBS_ReadHeader(SourceStream* stream, MBSPatchHeader* header) {
  if (!stream->Read(header->tag, sizeof(header->tag))) return READ_ERROR;
  if (!stream->ReadVarint32(&header->slen)) return READ_ERROR;
  if (!stream->ReadVarint32(&header->scrc32)) return READ_ERROR;
  if (!stream->ReadVarint32(&header->dlen)) return READ_ERROR;
  if (!stream->ReadVarint32(&header->cblen)) return READ_ERROR;
  if (!stream->ReadVarint32(&header->difflen)) return READ_ERROR;
  if (!stream->ReadVarint32(&header->extralen)) return READ_ERROR;

  // The string will have a NUL terminator that we don't use, hence '-1'.
  COMPILE_ASSERT(sizeof(MBS_PATCH_HEADER_TAG) - 1 == sizeof(header->tag),
                 MBS_PATCH_HEADER_TAG_must_match_header_field_size);
  if (memcmp(header->tag, MBS_PATCH_HEADER_TAG, 8) != 0)
    return UNEXPECTED_ERROR;

  size_t bytes_remaining = stream->Remaining();
  if (header->cblen +
      header->difflen +
      header->extralen != bytes_remaining)
    return UNEXPECTED_ERROR;

  return OK;
}

BSDiffStatus MBS_ApplyPatch(const MBSPatchHeader *header,
                            SourceStream* patch_stream,
                            const uint8* old_start, size_t old_size,
                            SinkStream* new_stream) {
  const uint8* old_end = old_start + old_size;

  SourceStream control_stream;

  const uint8* control_start = patch_stream->Buffer();
  if (!patch_stream->ReadSubstream(header->cblen, &control_stream))
    return READ_ERROR;
  if (!patch_stream->Skip(header->difflen + header->extralen))
    return READ_ERROR;
  if (!patch_stream->Empty())
    return READ_ERROR;

  const uint8* diff_start = control_start + header->cblen;
  const uint8* diff_end = diff_start + header->difflen;
  const uint8* extra_start = diff_end;
  const uint8* extra_end = extra_start + header->extralen;

  const uint8* old_position = old_start;
  const uint8* diff_position = diff_start;
  const uint8* extra_position = extra_start;

  new_stream->Reserve(header->dlen);

  while (!control_stream.Empty()) {
    uint32 copy_count, extra_count;
    int32 seek_adjustment;
    if (!control_stream.ReadVarint32(&copy_count))
      return UNEXPECTED_ERROR;
    if (!control_stream.ReadVarint32(&extra_count))
      return UNEXPECTED_ERROR;
    if (!control_stream.ReadVarint32Signed(&seek_adjustment))
      return UNEXPECTED_ERROR;

#ifdef DEBUG_bsmedberg
    printf("Applying block:  copy: %-8u extra: %-8u seek: %+i\n",
           copy_count, extra_count, seek_adjustment);
#endif
    // Byte-wise arithmetically add bytes from old file to bytes from the diff
    // block.
    if (copy_count > static_cast<size_t>(old_end - old_position))
      return UNEXPECTED_ERROR;
    if (copy_count > static_cast<size_t>(diff_end - diff_position))
      return UNEXPECTED_ERROR;

    // Add together bytes from the 'old' file and the 'diff' stream.
    for (size_t i = 0;  i < copy_count;  ++i) {
      uint8 byte = old_position[i] + diff_position[i];
      new_stream->Write(&byte, 1);
    }
    old_position += copy_count;
    diff_position += copy_count;

    // Copy bytes from the extra block.
    if (extra_count > static_cast<size_t>(extra_end - extra_position))
      return UNEXPECTED_ERROR;

    new_stream->Write(extra_position, extra_count);
    extra_position += extra_count;

    // "seek" forwards (or backwards) in oldfile.
    if (old_position + seek_adjustment < old_start ||
        old_position + seek_adjustment > old_end)
      return UNEXPECTED_ERROR;

    old_position += seek_adjustment;
  }

  if (diff_position != diff_end)
    return UNEXPECTED_ERROR;
  if (extra_position != extra_end)
    return UNEXPECTED_ERROR;

  return OK;
}

BSDiffStatus ApplyBinaryPatch(SourceStream* old_stream,
                              SourceStream* patch_stream,
                              SinkStream* new_stream) {
  MBSPatchHeader header;
  BSDiffStatus ret = MBS_ReadHeader(patch_stream, &header);
  if (ret != OK) return ret;

  const uint8* old_start = old_stream->Buffer();
  size_t old_size = old_stream->Remaining();

  if (old_size != header.slen) return UNEXPECTED_ERROR;

  if (CalculateCrc(old_start, old_size) != header.scrc32)
    return CRC_ERROR;

  MBS_ApplyPatch(&header, patch_stream, old_start, old_size, new_stream);

  return OK;
}

}  // namespace
