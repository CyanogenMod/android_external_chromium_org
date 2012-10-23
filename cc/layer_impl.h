// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCLayerImpl_h
#define CCLayerImpl_h

#include "FloatRect.h"
#include "IntRect.h"
#include "Region.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "cc/input_handler.h"
#include "cc/layer_animation_controller.h"
#include "cc/render_pass.h"
#include "cc/render_surface_impl.h"
#include "cc/resource_provider.h"
#include "cc/scoped_ptr_vector.h"
#include "cc/shared_quad_state.h"
#include "third_party/skia/include/core/SkColor.h"
#include <public/WebFilterOperations.h>
#include <public/WebTransformationMatrix.h>
#include <string>

namespace cc {

class LayerSorter;
class LayerTreeHostImpl;
class QuadSink;
class Renderer;
class ScrollbarAnimationController;
class ScrollbarLayerImpl;
class Layer;

struct AppendQuadsData;

class LayerImpl : public LayerAnimationControllerClient {
public:
    static scoped_ptr<LayerImpl> create(int id)
    {
        return make_scoped_ptr(new LayerImpl(id));
    }

    virtual ~LayerImpl();

    // LayerAnimationControllerClient implementation.
    virtual int id() const OVERRIDE;
    virtual void setOpacityFromAnimation(float) OVERRIDE;
    virtual float opacity() const OVERRIDE;
    virtual void setTransformFromAnimation(const WebKit::WebTransformationMatrix&) OVERRIDE;
    virtual const WebKit::WebTransformationMatrix& transform() const OVERRIDE;

    // Tree structure.
    LayerImpl* parent() const { return m_parent; }
    const ScopedPtrVector<LayerImpl>& children() const { return m_children; }
    void addChild(scoped_ptr<LayerImpl>);
    void removeFromParent();
    void removeAllChildren();

    void setMaskLayer(scoped_ptr<LayerImpl>);
    LayerImpl* maskLayer() const { return m_maskLayer.get(); }

    void setReplicaLayer(scoped_ptr<LayerImpl>);
    LayerImpl* replicaLayer() const { return m_replicaLayer.get(); }

    bool hasMask() const { return m_maskLayer; }
    bool hasReplica() const { return m_replicaLayer; }
    bool replicaHasMask() const { return m_replicaLayer && (m_maskLayer || m_replicaLayer->m_maskLayer); }

    LayerTreeHostImpl* layerTreeHostImpl() const { return m_layerTreeHostImpl; }
    void setLayerTreeHostImpl(LayerTreeHostImpl* hostImpl) { m_layerTreeHostImpl = hostImpl; }

    scoped_ptr<SharedQuadState> createSharedQuadState() const;
    // willDraw must be called before appendQuads. If willDraw is called,
    // didDraw is guaranteed to be called before another willDraw or before
    // the layer is destroyed. To enforce this, any class that overrides
    // willDraw/didDraw must call the base class version.
    virtual void willDraw(ResourceProvider*);
    virtual void appendQuads(QuadSink&, AppendQuadsData&) { }
    virtual void didDraw(ResourceProvider*);

    virtual ResourceProvider::ResourceId contentsResourceId() const;

    virtual bool hasContributingDelegatedRenderPasses() const;
    virtual RenderPass::Id firstContributingRenderPassId() const;
    virtual RenderPass::Id nextContributingRenderPassId(RenderPass::Id) const;

    // Returns true if this layer has content to draw.
    void setDrawsContent(bool);
    bool drawsContent() const { return m_drawsContent; }

    bool forceRenderSurface() const { return m_forceRenderSurface; }
    void setForceRenderSurface(bool force) { m_forceRenderSurface = force; }

    // Returns true if any of the layer's descendants has content to draw.
    virtual bool descendantDrawsContent();

    void setAnchorPoint(const FloatPoint&);
    const FloatPoint& anchorPoint() const { return m_anchorPoint; }

    void setAnchorPointZ(float);
    float anchorPointZ() const { return m_anchorPointZ; }

    void setBackgroundColor(SkColor);
    SkColor backgroundColor() const { return m_backgroundColor; }

    void setFilters(const WebKit::WebFilterOperations&);
    const WebKit::WebFilterOperations& filters() const { return m_filters; }

    void setBackgroundFilters(const WebKit::WebFilterOperations&);
    const WebKit::WebFilterOperations& backgroundFilters() const { return m_backgroundFilters; }

    void setMasksToBounds(bool);
    bool masksToBounds() const { return m_masksToBounds; }

    void setContentsOpaque(bool);
    bool contentsOpaque() const { return m_contentsOpaque; }

    void setOpacity(float);
    bool opacityIsAnimating() const;

    void setPosition(const FloatPoint&);
    const FloatPoint& position() const { return m_position; }

    void setIsContainerForFixedPositionLayers(bool isContainerForFixedPositionLayers) { m_isContainerForFixedPositionLayers = isContainerForFixedPositionLayers; }
    bool isContainerForFixedPositionLayers() const { return m_isContainerForFixedPositionLayers; }

    void setFixedToContainerLayer(bool fixedToContainerLayer = true) { m_fixedToContainerLayer = fixedToContainerLayer;}
    bool fixedToContainerLayer() const { return m_fixedToContainerLayer; }

    void setPreserves3D(bool);
    bool preserves3D() const { return m_preserves3D; }

    void setUseParentBackfaceVisibility(bool useParentBackfaceVisibility) { m_useParentBackfaceVisibility = useParentBackfaceVisibility; }
    bool useParentBackfaceVisibility() const { return m_useParentBackfaceVisibility; }

    void setUseLCDText(bool useLCDText) { m_useLCDText = useLCDText; }
    bool useLCDText() const { return m_useLCDText; }

    void setSublayerTransform(const WebKit::WebTransformationMatrix&);
    const WebKit::WebTransformationMatrix& sublayerTransform() const { return m_sublayerTransform; }

    // Debug layer border - visual effect only, do not change geometry/clipping/etc.
    void setDebugBorderColor(SkColor);
    SkColor debugBorderColor() const { return m_debugBorderColor; }
    void setDebugBorderWidth(float);
    float debugBorderWidth() const { return m_debugBorderWidth; }
    bool hasDebugBorders() const;

    // Debug layer name.
    void setDebugName(const std::string& debugName) { m_debugName = debugName; }
    std::string debugName() const { return m_debugName; }

    RenderSurfaceImpl* renderSurface() const { return m_renderSurface.get(); }
    void createRenderSurface();
    void clearRenderSurface() { m_renderSurface.reset(); }

    float drawOpacity() const { return m_drawOpacity; }
    void setDrawOpacity(float opacity) { m_drawOpacity = opacity; }

    bool drawOpacityIsAnimating() const { return m_drawOpacityIsAnimating; }
    void setDrawOpacityIsAnimating(bool drawOpacityIsAnimating) { m_drawOpacityIsAnimating = drawOpacityIsAnimating; }

    LayerImpl* renderTarget() const { DCHECK(!m_renderTarget || m_renderTarget->renderSurface()); return m_renderTarget; }
    void setRenderTarget(LayerImpl* target) { m_renderTarget = target; }

    void setBounds(const IntSize&);
    const IntSize& bounds() const { return m_bounds; }

    const IntSize& contentBounds() const { return m_contentBounds; }
    void setContentBounds(const IntSize&);

    const IntPoint& scrollPosition() const { return m_scrollPosition; }
    void setScrollPosition(const IntPoint&);

    const IntSize& maxScrollPosition() const {return m_maxScrollPosition; }
    void setMaxScrollPosition(const IntSize&);

    const FloatSize& scrollDelta() const { return m_scrollDelta; }
    void setScrollDelta(const FloatSize&);

    const WebKit::WebTransformationMatrix& implTransform() const { return m_implTransform; }
    void setImplTransform(const WebKit::WebTransformationMatrix& transform);

    const IntSize& sentScrollDelta() const { return m_sentScrollDelta; }
    void setSentScrollDelta(const IntSize& sentScrollDelta) { m_sentScrollDelta = sentScrollDelta; }

    // Returns the delta of the scroll that was outside of the bounds of the initial scroll
    FloatSize scrollBy(const FloatSize& scroll);

    bool scrollable() const { return m_scrollable; }
    void setScrollable(bool scrollable) { m_scrollable = scrollable; }

    bool shouldScrollOnMainThread() const { return m_shouldScrollOnMainThread; }
    void setShouldScrollOnMainThread(bool shouldScrollOnMainThread) { m_shouldScrollOnMainThread = shouldScrollOnMainThread; }

    bool haveWheelEventHandlers() const { return m_haveWheelEventHandlers; }
    void setHaveWheelEventHandlers(bool haveWheelEventHandlers) { m_haveWheelEventHandlers = haveWheelEventHandlers; }

    const Region& nonFastScrollableRegion() const { return m_nonFastScrollableRegion; }
    void setNonFastScrollableRegion(const Region& region) { m_nonFastScrollableRegion = region; }

    void setDrawCheckerboardForMissingTiles(bool checkerboard) { m_drawCheckerboardForMissingTiles = checkerboard; }
    bool drawCheckerboardForMissingTiles() const;

    InputHandlerClient::ScrollStatus tryScroll(const IntPoint& viewportPoint, InputHandlerClient::ScrollInputType) const;

    const IntRect& visibleContentRect() const { return m_visibleContentRect; }
    void setVisibleContentRect(const IntRect& visibleContentRect) { m_visibleContentRect = visibleContentRect; }

    bool doubleSided() const { return m_doubleSided; }
    void setDoubleSided(bool);

    void setTransform(const WebKit::WebTransformationMatrix&);
    bool transformIsAnimating() const;

    const WebKit::WebTransformationMatrix& drawTransform() const { return m_drawTransform; }
    void setDrawTransform(const WebKit::WebTransformationMatrix& matrix) { m_drawTransform = matrix; }
    const WebKit::WebTransformationMatrix& screenSpaceTransform() const { return m_screenSpaceTransform; }
    void setScreenSpaceTransform(const WebKit::WebTransformationMatrix& matrix) { m_screenSpaceTransform = matrix; }

    bool drawTransformIsAnimating() const { return m_drawTransformIsAnimating; }
    void setDrawTransformIsAnimating(bool animating) { m_drawTransformIsAnimating = animating; }
    bool screenSpaceTransformIsAnimating() const { return m_screenSpaceTransformIsAnimating; }
    void setScreenSpaceTransformIsAnimating(bool animating) { m_screenSpaceTransformIsAnimating = animating; }

    const IntRect& drawableContentRect() const { return m_drawableContentRect; }
    void setDrawableContentRect(const IntRect& rect) { m_drawableContentRect = rect; }
    const FloatRect& updateRect() const { return m_updateRect; }
    void setUpdateRect(const FloatRect& updateRect) { m_updateRect = updateRect; }

    std::string layerTreeAsText() const;

    void setStackingOrderChanged(bool);

    bool layerPropertyChanged() const { return m_layerPropertyChanged || layerIsAlwaysDamaged(); }
    bool layerSurfacePropertyChanged() const;

    void resetAllChangeTrackingForSubtree();

    virtual bool layerIsAlwaysDamaged() const;

    LayerAnimationController* layerAnimationController() { return m_layerAnimationController.get(); }

    virtual Region visibleContentOpaqueRegion() const;

    // Indicates that the context previously used to render this layer
    // was lost and that a new one has been created. Won't be called
    // until the new context has been created successfully.
    virtual void didLoseContext();

    ScrollbarAnimationController* scrollbarAnimationController() const { return m_scrollbarAnimationController.get(); }

    ScrollbarLayerImpl* horizontalScrollbarLayer() const;
    void setHorizontalScrollbarLayer(ScrollbarLayerImpl*);

    ScrollbarLayerImpl* verticalScrollbarLayer() const;
    void setVerticalScrollbarLayer(ScrollbarLayerImpl*);

protected:
    explicit LayerImpl(int);

    void appendDebugBorderQuad(QuadSink&, const SharedQuadState*, AppendQuadsData&) const;

    IntRect layerRectToContentRect(const WebKit::WebRect& layerRect);

    virtual void dumpLayerProperties(std::string*, int indent) const;
    static std::string indentString(int indent);

private:
    void setParent(LayerImpl* parent) { m_parent = parent; }
    friend class TreeSynchronizer;
    void clearChildList(); // Warning: This does not preserve tree structure invariants and so is only exposed to the tree synchronizer.

    void noteLayerPropertyChangedForSubtree();

    // Note carefully this does not affect the current layer.
    void noteLayerPropertyChangedForDescendants();

    virtual const char* layerTypeAsString() const;

    void dumpLayer(std::string*, int indent) const;

    // Properties internal to LayerImpl
    LayerImpl* m_parent;
    ScopedPtrVector<LayerImpl> m_children;
    // m_maskLayer can be temporarily stolen during tree sync, we need this ID to confirm newly assigned layer is still the previous one
    int m_maskLayerId;
    scoped_ptr<LayerImpl> m_maskLayer;
    int m_replicaLayerId; // ditto
    scoped_ptr<LayerImpl> m_replicaLayer;
    int m_layerId;
    LayerTreeHostImpl* m_layerTreeHostImpl;

    // Properties synchronized from the associated Layer.
    FloatPoint m_anchorPoint;
    float m_anchorPointZ;
    IntSize m_bounds;
    IntSize m_contentBounds;
    IntPoint m_scrollPosition;
    bool m_scrollable;
    bool m_shouldScrollOnMainThread;
    bool m_haveWheelEventHandlers;
    Region m_nonFastScrollableRegion;
    SkColor m_backgroundColor;

    // Whether the "back" of this layer should draw.
    bool m_doubleSided;

    // Tracks if drawing-related properties have changed since last redraw.
    bool m_layerPropertyChanged;

    // Indicates that a property has changed on this layer that would not
    // affect the pixels on its target surface, but would require redrawing
    // but would require redrawing the targetSurface onto its ancestor targetSurface.
    // For layers that do not own a surface this flag acts as m_layerPropertyChanged.
    bool m_layerSurfacePropertyChanged;

    // Uses layer's content space.
    IntRect m_visibleContentRect;
    bool m_masksToBounds;
    bool m_contentsOpaque;
    float m_opacity;
    FloatPoint m_position;
    bool m_preserves3D;
    bool m_useParentBackfaceVisibility;
    bool m_drawCheckerboardForMissingTiles;
    WebKit::WebTransformationMatrix m_sublayerTransform;
    WebKit::WebTransformationMatrix m_transform;
    bool m_useLCDText;

    bool m_drawsContent;
    bool m_forceRenderSurface;

    // Set for the layer that other layers are fixed to.
    bool m_isContainerForFixedPositionLayers;
    // This is true if the layer should be fixed to the closest ancestor container.
    bool m_fixedToContainerLayer;

    FloatSize m_scrollDelta;
    IntSize m_sentScrollDelta;
    IntSize m_maxScrollPosition;
    WebKit::WebTransformationMatrix m_implTransform;

    // The layer whose coordinate space this layer draws into. This can be
    // either the same layer (m_renderTarget == this) or an ancestor of this
    // layer.
    LayerImpl* m_renderTarget;

    // The global depth value of the center of the layer. This value is used
    // to sort layers from back to front.
    float m_drawDepth;
    float m_drawOpacity;
    bool m_drawOpacityIsAnimating;

    // Debug borders.
    SkColor m_debugBorderColor;
    float m_debugBorderWidth;

    // Debug layer name.
    std::string m_debugName;

    WebKit::WebFilterOperations m_filters;
    WebKit::WebFilterOperations m_backgroundFilters;

    WebKit::WebTransformationMatrix m_drawTransform;
    WebKit::WebTransformationMatrix m_screenSpaceTransform;
    bool m_drawTransformIsAnimating;
    bool m_screenSpaceTransformIsAnimating;

#ifndef NDEBUG
    bool m_betweenWillDrawAndDidDraw;
#endif

    // Render surface associated with this layer. The layer and its descendants
    // will render to this surface.
    scoped_ptr<RenderSurfaceImpl> m_renderSurface;

    // Hierarchical bounding rect containing the layer and its descendants.
    // Uses target surface's space.
    IntRect m_drawableContentRect;

    // Rect indicating what was repainted/updated during update.
    // Note that plugin layers bypass this and leave it empty.
    // Uses layer's content space.
    FloatRect m_updateRect;

    // Manages animations for this layer.
    scoped_ptr<LayerAnimationController> m_layerAnimationController;

    // Manages scrollbars for this layer
    scoped_ptr<ScrollbarAnimationController> m_scrollbarAnimationController;
};

void sortLayers(std::vector<LayerImpl*>::iterator first, std::vector<LayerImpl*>::iterator end, LayerSorter*);

}

#endif // CCLayerImpl_h
