/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/myst3/gfx.h"

#include "engines/util.h"

#include "common/config-manager.h"

#include "graphics/renderer.h"
#include "graphics/surface.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS)
#include "graphics/opengl/context.h"
#endif

#include "math/glmath.h"

namespace Myst3 {

const float Renderer::cubeVertices[] = {
	// S     T      X      Y      Z
	0.0f, 1.0f, -320.0f, -320.0f, -320.0f,
	1.0f, 1.0f,  320.0f, -320.0f, -320.0f,
	0.0f, 0.0f, -320.0f,  320.0f, -320.0f,
	1.0f, 0.0f,  320.0f,  320.0f, -320.0f,
	0.0f, 1.0f,  320.0f, -320.0f, -320.0f,
	1.0f, 1.0f, -320.0f, -320.0f, -320.0f,
	0.0f, 0.0f,  320.0f, -320.0f,  320.0f,
	1.0f, 0.0f, -320.0f, -320.0f,  320.0f,
	0.0f, 1.0f,  320.0f, -320.0f,  320.0f,
	1.0f, 1.0f, -320.0f, -320.0f,  320.0f,
	0.0f, 0.0f,  320.0f,  320.0f,  320.0f,
	1.0f, 0.0f, -320.0f,  320.0f,  320.0f,
	0.0f, 1.0f,  320.0f, -320.0f, -320.0f,
	1.0f, 1.0f,  320.0f, -320.0f,  320.0f,
	0.0f, 0.0f,  320.0f,  320.0f, -320.0f,
	1.0f, 0.0f,  320.0f,  320.0f,  320.0f,
	0.0f, 1.0f, -320.0f, -320.0f,  320.0f,
	1.0f, 1.0f, -320.0f, -320.0f, -320.0f,
	0.0f, 0.0f, -320.0f,  320.0f,  320.0f,
	1.0f, 0.0f, -320.0f,  320.0f, -320.0f,
	0.0f, 1.0f,  320.0f,  320.0f,  320.0f,
	1.0f, 1.0f, -320.0f,  320.0f,  320.0f,
	0.0f, 0.0f,  320.0f,  320.0f, -320.0f,
	1.0f, 0.0f, -320.0f,  320.0f, -320.0f
};

Renderer::Renderer(OSystem *system)
		: _system(system),
		  _font(nullptr) {
	// Compute the cube faces Axis Aligned Bounding Boxes
	for (uint i = 0; i < ARRAYSIZE(_cubeFacesAABB); i++) {
		for (uint j = 0; j < 4; j++) {
			_cubeFacesAABB[i].expand(Math::Vector3d(cubeVertices[5 * (4 * i + j) + 2], cubeVertices[5 * (4 * i + j) + 3], cubeVertices[5 * (4 * i + j) + 4]));
		}
	}
}

Renderer::~Renderer() {
}

void Renderer::initFont(const Graphics::Surface *surface) {
	_font = createTexture2D(surface);
}

void Renderer::freeFont() {
	if (_font) {
		delete _font;
		_font = nullptr;
	}
}

Texture *Renderer::copyScreenshotToTexture() {
	Graphics::Surface *surface = getScreenshot();

	Texture *texture = createTexture2D(surface);

	surface->free();
	delete surface;

	return texture;
}

Common::Rect Renderer::getFontCharacterRect(uint8 character) {
	uint index = 0;

	if (character == ' ')
		index = 0;
	else if (character >= '0' && character <= '9')
		index = 1 + character - '0';
	else if (character >= 'A' && character <= 'Z')
		index = 1 + 10 + character - 'A';
	else if (character == '|')
		index = 1 + 10 + 26;
	else if (character == '/')
		index = 2 + 10 + 26;
	else if (character == ':')
		index = 3 + 10 + 26;

	return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

Common::Rect Renderer::viewport() const {
	return _screenViewport;
}

void Renderer::computeScreenViewport() {
	int32 screenWidth = _system->getWidth();
	int32 screenHeight = _system->getHeight();

	if (ConfMan.getBool("widescreen_mod")) {
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	} else {
		// Aspect ratio correction
		int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * kOriginalWidth / kOriginalHeight);
		int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * kOriginalHeight / kOriginalWidth);
		_screenViewport = Common::Rect(viewportWidth, viewportHeight);

		// Pillarboxing
		_screenViewport.translate((screenWidth - viewportWidth) / 2,
			(screenHeight - viewportHeight) / 2);
	}
}

Math::Matrix4 Renderer::makeProjectionMatrix(float fov) const {
	static const float nearClipPlane = 1.0;
	static const float farClipPlane = 10000.0;

	float aspectRatio = kOriginalWidth / (float) kFrameHeight;

	float xmaxValue = nearClipPlane * tan(fov * M_PI / 360.0);
	float ymaxValue = xmaxValue / aspectRatio;

	return Math::makeFrustumMatrix(-xmaxValue, xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
}

void Renderer::setupCameraPerspective(float pitch, float heading, float fov) {
	_projectionMatrix = makeProjectionMatrix(fov);
	_modelViewMatrix = Math::Matrix4(180.0f - heading, pitch, 0.0f, Math::EO_YXZ);

	Math::Matrix4 proj = _projectionMatrix;
	Math::Matrix4 model = _modelViewMatrix;
	proj.transpose();
	model.transpose();

	_mvpMatrix = proj * model;

	_frustum.setup(_mvpMatrix);

	_mvpMatrix.transpose();
}

bool Renderer::isCubeFaceVisible(uint face) {
	assert(face < 6);

	return _frustum.isInside(_cubeFacesAABB[face]);
}

void Renderer::flipVertical(Graphics::Surface *s) {
	for (int y = 0; y < s->h / 2; ++y) {
		// Flip the lines
		byte *line1P = (byte *)s->getBasePtr(0, y);
		byte *line2P = (byte *)s->getBasePtr(0, s->h - y - 1);

		for (int x = 0; x < s->pitch; ++x)
			SWAP(line1P[x], line2P[x]);
	}
}

Renderer *createRenderer(OSystem *system) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::Renderer::getBestMatchingAvailableType(desiredRendererType,
#if defined(USE_OPENGL_GAME)
			Graphics::kRendererTypeOpenGL |
#endif
#if defined(USE_OPENGL_SHADERS)
			Graphics::kRendererTypeOpenGLShaders |
#endif
#if defined(USE_TINYGL)
			Graphics::kRendererTypeTinyGL |
#endif
			0);

	bool isAccelerated = matchingRendererType != Graphics::kRendererTypeTinyGL;

	uint width;
	uint height = Renderer::kOriginalHeight;
	if (ConfMan.getBool("widescreen_mod")) {
		width = Renderer::kOriginalWidth * Renderer::kOriginalHeight / Renderer::kFrameHeight;
	} else {
		width = Renderer::kOriginalWidth;
	}

	if (isAccelerated) {
		initGraphics3d(width, height);
	} else {
		initGraphics(width, height, nullptr);
	}

#if defined(USE_OPENGL_SHADERS)
	if (matchingRendererType == Graphics::kRendererTypeOpenGLShaders) {
		return CreateGfxOpenGLShader(system);
	}
#endif
#if defined(USE_OPENGL_GAME)
	if (matchingRendererType == Graphics::kRendererTypeOpenGL) {
		return CreateGfxOpenGL(system);
	}
#endif
#if defined(USE_TINYGL)
	if (matchingRendererType == Graphics::kRendererTypeTinyGL) {
		return CreateGfxTinyGL(system);
	}
#endif
	/* We should never end up here, getBestMatchingRendererType would have failed before */
	error("Unable to create a renderer");
}

void Renderer::renderDrawable(Drawable *drawable, Window *window) {
	if (drawable->isConstrainedToWindow()) {
		selectTargetWindow(window, drawable->is3D(), drawable->isScaled());
	} else {
		selectTargetWindow(nullptr, drawable->is3D(), drawable->isScaled());
	}
	drawable->draw();
}

void Renderer::renderDrawableOverlay(Drawable *drawable, Window *window) {
	// Overlays are always 2D
	if (drawable->isConstrainedToWindow()) {
		selectTargetWindow(window, drawable->is3D(), drawable->isScaled());
	} else {
		selectTargetWindow(nullptr, drawable->is3D(), drawable->isScaled());
	}
	drawable->drawOverlay();
}

void Renderer::renderWindow(Window *window) {
	renderDrawable(window, window);
}

void Renderer::renderWindowOverlay(Window *window) {
	renderDrawableOverlay(window, window);
}

Drawable::Drawable() :
		_isConstrainedToWindow(true),
		_is3D(false),
		_scaled(true) {
}

Common::Point Window::getCenter() const {
	Common::Rect frame = getPosition();

	return Common::Point((frame.left + frame.right) / 2, (frame.top + frame.bottom) / 2);
}

Common::Point Window::screenPosToWindowPos(const Common::Point &screen) const {
	Common::Rect frame = getPosition();

	return Common::Point(screen.x - frame.left, screen.y - frame.top);
}

Common::Point Window::scalePoint(const Common::Point &screen) const {
	Common::Rect viewport = getPosition();
	Common::Rect originalViewport = getOriginalPosition();

	Common::Point scaledPosition = screen;
	scaledPosition.x -= viewport.left;
	scaledPosition.y -= viewport.top;
	scaledPosition.x = CLIP<int16>(scaledPosition.x, 0, viewport.width());
	scaledPosition.y = CLIP<int16>(scaledPosition.y, 0, viewport.height());

	if (_scaled) {
		scaledPosition.x *= originalViewport.width() / (float) viewport.width();
		scaledPosition.y *= originalViewport.height() / (float) viewport.height();
	}

	return scaledPosition;
}

const Graphics::PixelFormat Texture::getRGBAPixelFormat() {
	return Graphics::PixelFormat::createFormatRGBA32();
}

} // End of namespace Myst3
