#include <graphics/Enum.h>

namespace hpl {

    static const uint32_t BlendSrcMask = 0x0F;
    static const uint32_t BlendDstMask = 0x0F;
    static const uint32_t BlendOpMask =  0x0F;

    static const uint32_t BlendSrcShift = 8;
    static const uint32_t BlendDstShift = 4;
    static const uint32_t BlendOpShift = 0;

    bool any(ClearOp write) {
        return static_cast<uint32_t>(write) != 0;
    }

    bool any(Write write) {
        return static_cast<uint32_t>(write) != 0;
    }

    Write operator|(Write lhs, Write rhs) {
        return static_cast<Write>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    } 

    Write operator&(Write lhs, Write rhs) {
        return static_cast<Write>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    ClearOp operator|(ClearOp lhs, ClearOp rhs) {
        return static_cast<ClearOp>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    ClearOp operator&(ClearOp lhs, ClearOp rhs) {
        return static_cast<ClearOp>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }


    BlendFunc CreateFromMaterialBlendMode(eMaterialBlendMode mode) {
        switch(mode) {
            case eMaterialBlendMode_Add:
                return CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::One);
            case eMaterialBlendMode_Mul:
                return CreateBlendFunction(BlendOperator::Add, BlendOperand::Zero, BlendOperand::SrcColor);
            case eMaterialBlendMode_MulX2:
                return CreateBlendFunction(BlendOperator::Add, BlendOperand::DstColor, BlendOperand::SrcColor);
            case eMaterialBlendMode_Alpha:
                return CreateBlendFunction(BlendOperator::Add, BlendOperand::SrcAlpha, BlendOperand::InvSrcAlpha);
            case eMaterialBlendMode_PremulAlpha:
                return CreateBlendFunction(BlendOperator::Add, BlendOperand::One, BlendOperand::InvSrcAlpha);
            default:
                break;
        }
        return BlendFunc(0); // empty
    }

    BlendFunc CreateBlendFunction(BlendOperator type, BlendOperand src, BlendOperand dst) {
        return static_cast<BlendFunc>(((static_cast<uint32_t>(type) & BlendOpMask) << BlendOpShift) | ((static_cast<uint32_t>(src) & BlendSrcMask) << BlendSrcShift) | ((static_cast<uint32_t>(dst) & BlendDstMask) << BlendDstShift));
    }

    BlendOperand GetBlendOperandSrc(BlendFunc func) {
        return static_cast<BlendOperand>((static_cast<uint32_t>(func) >> BlendSrcShift) & BlendSrcMask);
    }
    
    BlendOperand GetBlendOperandDst(BlendFunc func) {
        return static_cast<BlendOperand>((static_cast<uint32_t>(func) >> BlendDstShift) & BlendDstMask);
    }

    BlendOperator GetBlendOperator(BlendFunc func) {
        return static_cast<BlendOperator>((static_cast<uint32_t>(func) >> BlendOpShift) & BlendOpMask);
    }

    bool IsValidStencilTest(const StencilTest& test) {
        return test.m_func != StencilFunction::None;
    }

    StencilTest CreateStencilTest(StencilFunction func, StencilFail sfail, StencilDepthFail dpfail, StencilDepthPass dppass, uint8_t ref, uint8_t mask) {
        StencilTest test;
        test.m_func = func;
        test.m_sfail = sfail;
        test.m_dpfail = dpfail;
        test.m_dppass = dppass;
        test.m_ref = ref;
        test.m_mask = mask;
        return test;
    }
}