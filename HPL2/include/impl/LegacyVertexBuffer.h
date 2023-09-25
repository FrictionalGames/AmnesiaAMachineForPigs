/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <folly/small_vector.h>
#include <graphics/VertexBuffer.h>

#include <algorithm>
#include <array>
#include <span>
#include <vector>

#include <graphics/ForgeHandles.h>
#include <graphics/ForgeRenderer.h>
#include <Common_3/Graphics/Interfaces/IGraphics.h>
#include <FixPreprocessor.h>

namespace hpl {

    class LegacyVertexBuffer : public iVertexBuffer {
    public:
        static size_t GetSizeFromHPL(eVertexBufferElementFormat format);

        // static constexpr std::array

        struct VertexElement {
            SharedBuffer m_buffer;
            eVertexBufferElementFormat m_format = eVertexBufferElementFormat::eVertexBufferElementFormat_Float;
            eVertexBufferElement m_type = eVertexBufferElement::eVertexBufferElement_Position;
            tVertexElementFlag m_flag = 0;
            size_t m_num = 0;
            int m_programVarIndex = 0; // for legacy behavior

            size_t Stride() const;
            size_t NumElements() const;

            template<typename TData>
            std::span<TData> GetElements() {
                ASSERT(sizeof(TData) == Stride() && "Data must be same size as stride");
                return std::span<TData*>(reinterpret_cast<TData*>(m_shadowData.data()), m_shadowData.size() / Stride());
            }

            std::span<uint8_t> Data() const {
                return m_shadowData;
            }

            template<typename TData>
            TData& GetElement(size_t index) {
                ASSERT(sizeof(TData) <= Stride() && "Date must be less than or equal to stride");
                return *reinterpret_cast<TData*>(m_shadowData.data() + index * Stride());
            }

        private:
            mutable size_t m_activeCopy = 0; // the active copy of the data
            mutable size_t m_internalBufferSize = 0; // the size of the internal buffer
            mutable std::vector<uint8_t> m_shadowData = {};
            friend class LegacyVertexBuffer;
        };

        LegacyVertexBuffer(eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType, int alReserveVtxSize, int alReserveIdxSize);
        ~LegacyVertexBuffer();

        virtual void CreateElementArray(
            eVertexBufferElement aType, eVertexBufferElementFormat aFormat, int alElementNum, int alProgramVarIndex = 0) override;

        virtual void AddVertexVec3f(eVertexBufferElement aElement, const cVector3f& avVtx) override;
        virtual void AddVertexVec4f(eVertexBufferElement aElement, const cVector3f& avVtx, float afW) override;
        virtual void AddVertexColor(eVertexBufferElement aElement, const cColor& aColor) override;
        virtual void AddIndex(unsigned int alIndex) override;

        virtual bool Compile(tVertexCompileFlag aFlags) override;
        virtual void UpdateData(tVertexElementFlag aTypes, bool abIndices) override;

        virtual void Transform(const cMatrixf& mtxTransform) override;

        virtual void Draw(eVertexBufferDrawType aDrawType = eVertexBufferDrawType_LastEnum) override;

        struct GeometryBinding {
            struct VertexGeometryEntry {
                VertexElement* element;
                uint64_t offset;
            };
            struct VertexIndexEntry {
                SharedBuffer* element;
                uint64_t offset;
                uint32_t numIndicies;
            };
            folly::small_vector<VertexGeometryEntry, eVertexBufferElement_LastEnum> m_vertexElement; // elements are in the order they are requested
            VertexIndexEntry m_indexBuffer;
        };
        static void cmdBindGeometry(Cmd* cmd, ForgeRenderer::CommandResourcePool* resourcePool, LegacyVertexBuffer::GeometryBinding& binding);
        void resolveGeometryBinding(uint32_t currentFrame, std::span<eVertexBufferElement> elements, GeometryBinding* binding);

        virtual void UnBind() override;

        virtual iVertexBuffer* CreateCopy(
            eVertexBufferType aType, eVertexBufferUsageType aUsageType, tVertexElementFlag alVtxToCopy) override;

        virtual cBoundingVolume CreateBoundingVolume() override;

        virtual int GetVertexNum() override;
        virtual int GetIndexNum() override;

        virtual int GetElementNum(eVertexBufferElement aElement) override;
        virtual eVertexBufferElementFormat GetElementFormat(eVertexBufferElement aElement) override;
        virtual int GetElementProgramVarIndex(eVertexBufferElement aElement) override;

        virtual float* GetFloatArray(eVertexBufferElement aElement) override;
        virtual int* GetIntArray(eVertexBufferElement aElement) override;
        virtual unsigned char* GetByteArray(eVertexBufferElement aElement) override;

        virtual unsigned int* GetIndices() override;

        virtual void ResizeArray(eVertexBufferElement aElement, int alSize) override;
        virtual void ResizeIndices(int alSize) override;

        const VertexElement* GetElement(eVertexBufferElement elementType);

    protected:
        static void PushVertexElements(
            std::span<const float> values, eVertexBufferElement elementType, std::span<LegacyVertexBuffer::VertexElement> elements);

        folly::small_vector<VertexElement, 10> m_vertexElements = {};
        SharedBuffer m_indexBuffer;
        std::vector<uint32_t> m_indices = {};
        SyncToken m_bufferSync = {};

        size_t m_indexBufferActiveCopy = 0;
        tVertexElementFlag m_updateFlags = 0; // update no need to rebuild buffers
        bool m_updateIndices = false;

        friend struct VertexElement;
    };

}; // namespace hpl
