#include "nav_file.h"
#include <iostream>

namespace nav_mesh {
	nav_file::nav_file(std::string_view nav_mesh_file) {

		load(nav_mesh_file);
	}

	void nav_file::load(std::string_view nav_mesh_file) {
		if (!m_pather)
			m_pather = std::make_unique< micropather::MicroPather >(this);

		m_pather->Reset();
		m_areas.clear();
		m_places.clear();

		m_buffer.load_from_file(nav_mesh_file);

		if (m_buffer.read< std::uint32_t >() != m_magic)
			throw std::exception("nav_file::load: magic mismatch");

		m_version = m_buffer.read< std::uint32_t >();

		if (m_version != 16)
			throw std::exception("nav_file::load: version mismatch");

		m_sub_version = m_buffer.read< std::uint32_t >();
		m_source_bsp_size = m_buffer.read< std::uint32_t >();
		m_is_analyzed = m_buffer.read< std::uint8_t >();
		m_place_count = m_buffer.read< std::uint16_t >();

		for (std::uint16_t i = 0; i < m_place_count; i++) {
			auto place_name_length = m_buffer.read< std::uint16_t >();
			std::string place_name(place_name_length, 0);

			m_buffer.read(place_name.data(), place_name_length);
			m_places.push_back(place_name);
		}

		m_has_unnamed_areas = m_buffer.read< std::uint8_t >() != 0;
		m_area_count = m_buffer.read< std::uint32_t >();

		if (m_area_count == 0)
			throw std::exception("nav_file::load: no areas");

		for (std::uint32_t i = 0; i < m_area_count; i++) {
			nav_area area(m_buffer);
			
			m_areas.push_back(area);
		}

		if (m_areas.empty())
			throw std::exception("nav_file::load: No Areas loaded!");
		
		m_buffer.clear();
	}

	std::vector< vec3_t > nav_file::find_path(vec3_t from, vec3_t to) {
		auto start = reinterpret_cast<void*>(get_area_by_position(from).get_id());
		auto end = reinterpret_cast<void*>(get_area_by_position(to).get_id());

		float total_cost = 0.f;
		micropather::MPVector< void* > path_area_ids = { };

		if (m_pather->Solve(start, end, &path_area_ids, &total_cost) != 0)
			throw std::exception("nav_file::find_path: couldn't find path");

		std::vector< vec3_t > path = { };
		for (std::size_t i = 0; i < path_area_ids.size(); i++) {
			for (auto& area : m_areas) {
				if (area.get_id() == std::uint32_t(path_area_ids[i])) {
					path.push_back(area.get_center());
					break;
				}
			}
		}




		

		return path;
	}

	std::vector< vec3_t > nav_file::find_path(vec3_t from, int to_ID) {
		auto start = reinterpret_cast<void*>(get_area_by_position(from).get_id());
		auto end = reinterpret_cast<void*>(get_area_by_id((uint32_t)to_ID).get_id());

		float total_cost = 0.f;
		micropather::MPVector< void* > path_area_ids = { };

		if (m_pather->Solve(start, end, &path_area_ids, &total_cost) != 0)
			throw std::exception("nav_file::find_path: couldn't find path");

		std::vector< vec3_t > path = { };
		for (std::size_t i = 0; i < path_area_ids.size(); i++) {
			for (auto& area : m_areas) {
				if (area.get_id() == std::uint32_t(path_area_ids[i])) {
					path.push_back(area.get_center());
					break;
				}
			}
		}

		return path;
	}


	nav_area& nav_file::get_area_by_id(std::uint32_t id) {
		if (m_areas.empty()) {
			throw std::exception("NFE nav_file::get_area_by_id: nav_area (m_areas) empty");
		}

		for (auto& area : m_areas) {
			if (area.get_id() == id)
				return area;
		}

		throw std::exception("FTFA nav_file::get_area_by_id: failed to find area");
	}

	


	nav_area& nav_file::get_area_by_position(vec3_t position) {
		if (m_areas.empty()) {
			throw std::exception("NFE nav_file::get_area_by_position: nav_area (m_areas) empty");
		}

		for (auto& area : m_areas) {
			if (area.is_within(position))
				return area;
		}

		throw std::exception("FTFA nav_file::get_area_by_position: failed to find area");
	}

	nav_area& nav_file::get_area_by_position(Vector vPosition) {
		vec3_t position;
		position = position.convertSDKVector(vPosition);
		if (m_areas.empty()) {
			throw std::exception("NFE nav_file::get_area_by_position: nav_area (m_areas) empty");
		}

		for (auto& area : m_areas) {
			if (area.is_within(position))
				return area;
		}

		throw std::exception("FTFA nav_file::get_area_by_position: failed to find area");
	}
}