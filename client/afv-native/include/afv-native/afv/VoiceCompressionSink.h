/* afv/VoiceCompressionSink.h
 *
 * This file is part of AFV-Native.
 *
 * Copyright (c) 2019 Christopher Collins
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AFV_NATIVE_VOICECOMPRESSIONSINK_H
#define AFV_NATIVE_VOICECOMPRESSIONSINK_H

#include <opus/include/opus.h>
#include <vector>

#include "afv-native/audio/ISampleSink.h"

namespace afv_native {
    namespace afv {
        class ICompressedFrameSink {
        public:
            virtual void processCompressedFrame(std::vector<unsigned char> compressedData) = 0;
        };

        /** VoiceCompressionSink is an SampleSink that accepts samples from an origin and
         * encodes them via opus, and hands them to the next layer in the mess.
         */
        class VoiceCompressionSink: public audio::ISampleSink {
        protected:
            OpusEncoder *mEncoder;
            ICompressedFrameSink &mCompressedFrameSink;
        public:
            VoiceCompressionSink(ICompressedFrameSink &sink);
            virtual ~VoiceCompressionSink();
            int open();
            void close();
            void reset();
            void putAudioFrame(const audio::SampleType *bufferIn) override;
        };
    }
}


#endif //AFV_NATIVE_VOICECOMPRESSIONSINK_H
