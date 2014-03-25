/*Copyright (c) 2014, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DependentIOBufferWithSize_H
#define DependentIOBufferWithSize_H

#include "net/base/io_buffer.h"

namespace net {
/** a helper to connect onto an exiting IOBuffer at a given offset
 *
 * Connect to an existing buffer while maintaining the semantics of the IOBuffer class.
 *
 * note that the name is dependAnt rather than dependEnt.
 * \sa    content/browser/loader/buffered_resource_handler.cc#64
 */
class DependantIOBufferWithSize : public net::WrappedIOBuffer {
public:

    DependantIOBufferWithSize(): net::WrappedIOBuffer(NULL), size_(-1){}

    /** \brief Connect this object to en existing IOBuffer object (with allocated memory)
     *
     * \param backing   [in]  The buffer we hang on to
     * \param offset    [in]  byte offset in this buffer (zero based). Our zero offset is at this location
     * \param size      [in]  number of bytes "belonging" to this segment in the buffer.
     *                          It is the caller's responsibility to provide valid values.
     */
    void Connect(net::IOBufferWithSize* backing, int offset, int size){
        DCHECK(backing);
        DCHECK(offset >= 0);
        DCHECK(size >= 0);
        buf_ = backing;
        data_ = backing->data() + offset;
        size_ = size;
    }

    bool IsValid() { return size_ >=0 &&(data() != NULL);}

    int size() { return size_;}

private:

    virtual ~DependantIOBufferWithSize() {
        // we did not allocate buffer, so nothing to deallocate here.
        // Just make sure that my super class will not try to delete this data pointer (should have been a weak ptr).
        // The actual buffer is in buf_
        data_ = NULL;
    }

    scoped_refptr<net::IOBufferWithSize> buf_;

    /// size (in bytes) of the sub-buffer imposed on top of buf_
    int   size_;
};

}// namespace net

#endif

