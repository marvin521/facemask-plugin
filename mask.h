/*
 * Face Masks for SlOBS
 * Copyright (C) 2017 General Workings Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once
#include "mask-resource.h"
#include "mask-instance-data.h"
#include "mask-resource-morph.h"
#include "smll/TriangulationResult.hpp"
#include <string>
#include <vector>
#include <memory>
extern "C" {
	#pragma warning( push )
	#pragma warning( disable: 4201 )
	#include <libobs/obs-data.h>
	#include <libobs/graphics/vec3.h>
	#include <libobs/graphics/matrix4.h>
	#include <libobs/graphics/quat.h>
	#include <libobs/graphics/graphics.h>
	#pragma warning( pop )
}

namespace Mask {

	class MaskData;

	// Part : scene graph node
	// - keeps a local & global transform
	// - can be euler or quaternion rotations
	class Part : public Resource::IAnimatable {
	public:
		Part(std::shared_ptr<Part> p_parent,
			std::shared_ptr<Resource::IBase> p_resource);
		Part(std::shared_ptr<Resource::IBase> p_resource) :
			Part(nullptr, p_resource) {}
		Part(std::shared_ptr<Part> p_parent) : Part(p_parent, nullptr) {}
		Part() : Part(nullptr, nullptr) {}

		// IAnimatable
		void SetAnimatableValue(float v, 
			Resource::AnimationChannelType act) override;

		std::vector<std::shared_ptr<Resource::IBase>> resources;
		std::shared_ptr<Part> parent;
		std::size_t hash_id;
		vec3 position, scale, rotation;
		quat qrotation;

		// Internal
		matrix4 local, global;
		bool localdirty;
		bool dirty;
		bool isquat;
	};

	class SortedDrawObject {
	public:

		virtual ~SortedDrawObject() {}

		virtual float	SortDepth() = 0;
		virtual void	SortedRender() = 0;

		Part*				sortDrawPart;
		SortedDrawObject*	nextDrawObject;
		size_t				instanceId;
	};

	namespace Resource {
		class Animation;
	}

	class MaskData : public Resource::IAnimationControls {
	public:
		MaskData();
		virtual ~MaskData();

		// data
		void Clear();
		void Load(const std::string& file);

		// resources
		void AddResource(const std::string& name, std::shared_ptr<Resource::IBase> resource);
		std::shared_ptr<Resource::IBase> GetResource(const std::string& name);
		std::shared_ptr<Resource::IBase> GetResource(Resource::Type type);
		size_t GetNumResources(Resource::Type type);
		std::shared_ptr<Resource::IBase> GetResource(Resource::Type type, int which);
		std::shared_ptr<Resource::IBase> RemoveResource(const std::string& name);

		// parts
		void AddPart(const std::string& name, std::shared_ptr<Part> part);
		std::shared_ptr<Part> GetPart(const std::string& name);
		std::shared_ptr<Part> RemovePart(const std::string& name);
		size_t GetNumParts();
		std::shared_ptr<Part> GetPart(int index);

		// main: tick & render
		void Tick(float time);
		void Render(bool depthOnly = false);

		// IAnimationControls
		void	Play() override;
		void	PlayBackwards() override;
		void	Stop() override;
		void	Rewind(bool last = false) override;
		float	GetDuration() override;
		float	LastFrame() override;
		float	GetFPS() override;
		float	GetPlaybackSpeed() override;
		void	SetPlaybackSpeed(float speed) override;
		void    Seek(float time) override;
		bool	GetStopOnLastFrame() override;
		void	SetStopOnLastFrame(bool stop = true) override;
		float	GetPosition() override;

		// Global alpha
		float	GetGlobalAlpha();
		void	SetGlobalAlpha(float alpha);

		// sorted draw objects
		void ClearSortedDrawObjects();
		void AddSortedDrawObject(SortedDrawObject* obj);

		// morphs
		Resource::Morph*	 GetMorph();
		bool RenderMorphVideo(gs_texture* vidtex, uint32_t width, uint32_t height,
			const smll::TriangulationResult& trires);

		// intro animations
		bool	IsIntroAnimation() { return m_isIntroAnim; }
		float	GetIntroFadeTime() { return m_introFadeTime; }
		float	GetIntroDuration() { return m_introDuration; }

		// rendering flags
		bool	DrawVideoWithMask() { return m_drawVideoWithMask; }

		// global instance datas
		MaskInstanceDatas	instanceDatas;
		void				ResetInstanceDatas();

	private:
		std::shared_ptr<Part> LoadPart(std::string name, obs_data_t* data);
		static void PartCalcMatrix(std::shared_ptr<Mask::Part> part);

		struct {
			std::string name;
			std::string description;
			std::string author;
			std::string website;
		} m_metaData;
		std::map<std::string, std::shared_ptr<Resource::IBase>> m_resources;
		std::map<std::string, std::shared_ptr<Part>> m_parts;
		std::map<std::string, std::shared_ptr<Resource::Animation>> m_animations;
		obs_data_t* m_data;
		std::shared_ptr<Mask::Part> m_partWorld;
		SortedDrawObject**	m_drawBuckets;
		Resource::Morph*	m_morph;

		// for certain masks that need to blend with video
		bool				m_drawVideoWithMask;

		// for intro animation fading
		bool				m_isIntroAnim;
		float				m_introFadeTime;
		float				m_introDuration;
		float				m_elapsedTime;
	};
}
