/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef KTXREADER_KTX2READER_H
#define KTXREADER_KTX2READER_H

#include <cstdint>
#include <cstddef>

#include <filament/Texture.h>

#include <utils/FixedCapacityVector.h>

namespace filament {
    class Engine;
}

namespace basist {
    class ktx2_transcoder;
}

namespace ktxreader {

class Ktx2Reader {
    public:
        using Engine = filament::Engine;
        using Texture = filament::Texture;
        enum class TransferFunction { LINEAR, sRGB };

        enum class Result {
            SUCCESS,
            COMPRESSED_TRANSCODE_FAILURE,
            UNCOMPRESSED_TRANSCODE_FAILURE,
            FORMAT_UNSUPPORTED,
            FORMAT_ALREADY_REQUESTED,
        };

        Ktx2Reader(Engine& engine, bool quiet = false);
        ~Ktx2Reader();

        /**
         * Requests that the reader constructs Filament textures with given internal format.
         *
         * This MUST be called at least once before calling load().
         *
         * As a reminder, a basis-encoded KTX2 can be quickly transcoded to any number of formats,
         * so you need to tell it what formats your hw supports. That's why this method exists.
         *
         * Call requestFormat as many times as needed; formats that are submitted early are
         * considered higher priority.
         *
         * If BasisU knows a priori that the given format is not available (e.g. if the build has
         * disabled it), the format is not added and FORMAT_ALREADY_REQUESTED is returned.
         *
         * Returns FORMAT_ALREADY_REQUESTED if the given format has already been requested.
         *
         * Hint: BasisU supports the following uncompressed formats: RGBA8, RGB565, RGBA4.
         */
        Result requestFormat(Texture::InternalFormat format) noexcept;

        /**
         * Removes the given format from the list, or does nothing if it hasn't been requested.
         */
        void unrequestFormat(Texture::InternalFormat format) noexcept;

        /**
         * Attempts to create and load a Filament texture from the given KTX2 blob.
         *
         * If none of the requested formats can be extracted from the data, this returns null.
         *
         * This method iterates through the requested format list, checking each one against the
         * platform's capabilities and its availability from the transcoder. When a suitable format
         * is determined, it then performs lossless decompression (zstd) before transcoding the data
         * into the final format.
         *
         * The transfer function specified here is used in two ways:
         *   1) It is checked against the transfer function that was specified as metadata
         *      in the KTX2 blob. If they do not match, this method fails.
         *   2) It is used as a filter when determining the final internal format.
         */
        Texture* load(const uint8_t* data, size_t size, TransferFunction transfer);

        /**
         * Asynchronous Interface
         * ======================
         *
         * Alternative API suitable for asynchronous transcoding of mipmap levels.
         * If unsure that you need to use this, then don't, just call load() instead.
         * Usage pseudocode:
         *
         *    auto async = reader->asyncCreate(data, size, TransferFunction::LINEAR);
         *    mTexture = async->getTexture();
         *    auto backgroundThread = spawnThread({ async->doTranscoding(); })
         *    backgroundThread.wait();
         *    async->uploadImages();
         *    reader->asyncDestroy(async);
         *
         * In the documentation comments, "foreground thread" refers to the thread that the
         * Filament Engine was created on.
         */
        class Async {
        public:
            /**
             * Retrieves the Texture object.
             *
             * The texture is available immediately, but does not have its miplevels ready until
             * after uploadImages(). The caller has ownership over this texture and is responsible
             * for freeing it.
             */
            Texture* getTexture() const noexcept;

            /**
             * Loads mipmaps from the KTX2 file and transcodes them to the resolved format.
             *
             * This is typically called from a background thread.
             */
            Result doTranscoding();

            /**
             * Uploads all mipmaps to the texture.
             *
             * This calls Texture::setImage() and potentially generates missing mipmaps.
             * If transcoding has not yet completed, returns INCOMPLETE.
             *
             * NOTE: This can be called only from the foreground thread.
             */
            void uploadImages();

        protected:
            Async() noexcept = default;
            ~Async() = default;

        public:
            Async(Async const&) = delete;
            Async(Async&&) = delete;
            Async& operator=(Async const&) = delete;
            Async& operator=(Async&&) = delete;

            friend class Ktx2Reader;
        };

        /**
         * Creates a texture synchronously, but does not perform transcoding.
         *
         * The reader creates a copy of the given buffer so it can be freed immediately.
         * Callable from the foreground thread. If an error occurs, this returns null.
         * See load() documentation for additional details.
         */
        Async* asyncCreate(const uint8_t* data, size_t size, TransferFunction transfer);

        /**
         * Frees the given async object if it is non-null. (does not free the associated Texture)
         *
         * Callable from the foreground thread and assumes transcoding is not underway.
         */
        void asyncDestroy(Async* async);

    private:
        Ktx2Reader(const Ktx2Reader&) = delete;
        Ktx2Reader& operator=(const Ktx2Reader&) = delete;
        Ktx2Reader(Ktx2Reader&& that) noexcept = delete;
        Ktx2Reader& operator=(Ktx2Reader&& that) noexcept = delete;

        Texture* createTexture(basist::ktx2_transcoder* transcoder, const uint8_t* data,
                size_t size, TransferFunction transfer);

        Engine& mEngine;
        bool mQuiet;
        std::unique_ptr<basist::ktx2_transcoder> const mTranscoder;
        utils::FixedCapacityVector<Texture::InternalFormat> mRequestedFormats;
};

} // namespace ktxreader

#endif
