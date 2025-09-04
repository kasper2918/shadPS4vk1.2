#pragma once

#include <stdexcept>
#include <vector>
#include <cassert>

#include "common/types.h"
#include "video_core/renderer_vulkan/vk_common.h"

class Kasper {
public:
    static inline vk::RenderPass CreateDefaultRenderPass(vk::Device device, vk::Format format) {
        vk::AttachmentDescription color_attachment{};
        color_attachment.format = format;
        color_attachment.samples = vk::SampleCountFlagBits::e1;
        color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
        color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
        color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        color_attachment.initialLayout = vk::ImageLayout::eUndefined;
        color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        vk::RenderPass render_pass{};
        vk::RenderPassCreateInfo render_pass_info{};
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        if (device.createRenderPass(&render_pass_info, nullptr, &render_pass) !=
            vk::Result::eSuccess) {
            throw std::runtime_error{"Couldn't create default render pass"};
        }

        return render_pass;
    }

    static inline void PipelineBarrier1(
        const vk::CommandBuffer cmdbuf,
        const std::vector<vk::BufferMemoryBarrier2>& buffer_barriers2,
        const std::vector<vk::ImageMemoryBarrier2>& image_barriers2,
        vk::DependencyFlags flags = vk::DependencyInfo{}.dependencyFlags) {

        assert(buffer_barriers2.size() + image_barriers2.size() > 0 &&
               "Kasper::PipelineBarrier1: Barriers are empty");

        vk::PipelineStageFlags src_stage_mask{};
        vk::PipelineStageFlags dst_stage_mask{};

        const auto buffer_barriers1 =
            CreateBufferMemoryBarriers1(buffer_barriers2, src_stage_mask, dst_stage_mask);
        const auto image_barriers1 =
            CreateImageMemoryBarriers1(image_barriers2, src_stage_mask, dst_stage_mask);
        cmdbuf.pipelineBarrier(src_stage_mask, dst_stage_mask, flags, {}, buffer_barriers1,
                               image_barriers1);
    }

private:
    static inline std::vector<vk::BufferMemoryBarrier> CreateBufferMemoryBarriers1(
        const std::vector<vk::BufferMemoryBarrier2>& buffer_barriers2,
        vk::PipelineStageFlags& src_stage_mask, vk::PipelineStageFlags& dst_stage_mask) {

        std::vector<vk::BufferMemoryBarrier> buffer_barriers1{};
        buffer_barriers1.reserve(buffer_barriers2.size());

        for (const auto& barrier2 : buffer_barriers2) {
            src_stage_mask |= vk::PipelineStageFlags(u64(barrier2.srcStageMask));
            dst_stage_mask |= vk::PipelineStageFlags(u64(barrier2.dstStageMask));
            buffer_barriers1.emplace_back(vk::BufferMemoryBarrier{
                .pNext = barrier2.pNext,
                .srcAccessMask = vk::AccessFlags(u64(barrier2.srcAccessMask)),
                .dstAccessMask = vk::AccessFlags(u64(barrier2.dstAccessMask)),
                .srcQueueFamilyIndex = barrier2.srcQueueFamilyIndex,
                .dstQueueFamilyIndex = barrier2.dstQueueFamilyIndex,
                .buffer = barrier2.buffer,
                .offset = barrier2.offset,
                .size = barrier2.size});
        }

        return buffer_barriers1;
    }

    static inline std::vector<vk::ImageMemoryBarrier> CreateImageMemoryBarriers1(
        const std::vector<vk::ImageMemoryBarrier2>& image_barriers2,
        vk::PipelineStageFlags& src_stage_mask, vk::PipelineStageFlags& dst_stage_mask) {

        std::vector<vk::ImageMemoryBarrier> image_barriers1{};
        image_barriers1.reserve(image_barriers2.size());

        for (const auto& barrier2 : image_barriers2) {
            src_stage_mask |= vk::PipelineStageFlags(u64(barrier2.srcStageMask));
            dst_stage_mask |= vk::PipelineStageFlags(u64(barrier2.dstStageMask));
            image_barriers1.emplace_back(vk::ImageMemoryBarrier{
                .pNext = barrier2.pNext,
                .srcAccessMask = vk::AccessFlags(u64(barrier2.srcAccessMask)),
                .dstAccessMask = vk::AccessFlags(u64(barrier2.dstAccessMask)),
                .oldLayout = barrier2.oldLayout,
                .newLayout = barrier2.newLayout,
                .srcQueueFamilyIndex = barrier2.srcQueueFamilyIndex,
                .dstQueueFamilyIndex = barrier2.dstQueueFamilyIndex,
                .image = barrier2.image,
                .subresourceRange = barrier2.subresourceRange});
        }

        return image_barriers1;
    }
}; // class Kasper