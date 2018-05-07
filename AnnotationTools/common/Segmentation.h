#pragma once
#include "json.h"


struct SegmentationParams {
	float kThresh;
	unsigned int segMinVerts;
	unsigned int minPoints;
	unsigned int maxPoints;
	float thinThresh;
	float flatThresh;
	float minLength;
	float maxLength;

	SegmentationParams() {
		kThresh = 0.0f;
		segMinVerts = 0;
		minPoints = 0;
		maxPoints = 0;
		thinThresh = 0.0f;
		flatThresh = 0.0f;
		minLength = 0.0f;
		maxLength = 0.0f;
	}

	std::ostream& toJSON(std::ostream& os) const {
		os << "{" << std::endl;
		os << "\t\"kThresh\": \"" << kThresh << "\"," << std::endl;
		os << "\t\"segMinVerts\": \"" << segMinVerts << "\"," << std::endl;
		os << "\t\"minPoints\": \"" << minPoints << "\"," << std::endl;
		os << "\t\"maxPoints\": \"" << maxPoints << "\"," << std::endl;
		os << "\t\"thinThresh\": \"" << thinThresh << "\"," << std::endl;
		os << "\t\"flatThresh\": \"" << flatThresh << "\"," << std::endl;
		os << "\t\"minLength\": \"" << minLength << "\"," << std::endl;
		os << "\t\"maxLength\": \"" << maxLength << "\"" << std::endl;
		os << "}" << std::endl;
		return os;
	}
};

class Segmentation {
public:
	Segmentation() {}
	Segmentation(const std::string& filename) { loadFromFile(filename); }
	Segmentation(const std::vector<unsigned int>& ids, const std::string& sceneId = "");
	~Segmentation() {}

	void setSceneId(const std::string& sceneId) { m_sceneName = sceneId; }
	void copyParamsFrom(const Segmentation& other) {
		m_params = other.m_params;
	}

	void loadFromFile(const std::string& filename)
	{
		if (!ml::util::fileExists(filename)) throw MLIB_EXCEPTION("[parse] failed to open file " + filename);
		clear();

		// Parse JSON document
		rapidjson::Document d;
		if (!json::parseRapidJSONDocument(filename, &d)) {
			std::cerr << "Parse error reading " << filename << std::endl
				<< "Error code " << d.GetParseError() << " at " << d.GetErrorOffset() << std::endl;
			return;
		}

		m_sceneName = d["sceneId"].GetString();

		const auto& segIndices = d["segIndices"];
		m_segmentIds.resize(segIndices.Size());
		for (unsigned int i = 0; i < m_segmentIds.size(); i++) {
			m_segmentIds[i] = getUINT(segIndices[i]);
			auto it = m_segIdsToVertIds.find(m_segmentIds[i]);
			if (it == m_segIdsToVertIds.end()) m_segIdsToVertIds[m_segmentIds[i]] = std::vector<unsigned int>(1, i);
			else it->second.push_back(i);
		}

		if (d.HasMember("params")) {
			const auto& params = d["params"];
			m_params.kThresh = params.HasMember("kThresh") ? getFloat(params["kThresh"]) : 0.0f;
			m_params.segMinVerts = params.HasMember("segMinVerts") ? getUINT(params["segMinVerts"]) : 0.0f;
			m_params.minPoints = params.HasMember("minPoints") ? getUINT(params["minPoints"]) : 0.0f;
			m_params.maxPoints = params.HasMember("maxPoints") ? getUINT(params["maxPoints"]) : 0.0f;
			m_params.thinThresh = params.HasMember("thinThresh") ? getFloat(params["thinThresh"]) : 0.0f;
			m_params.flatThresh = params.HasMember("flatThresh") ? getFloat(params["flatThresh"]) : 0.0f;
			m_params.minLength = params.HasMember("minLength") ? getFloat(params["minLength"]) : 0.0f;
			m_params.maxLength = params.HasMember("maxLength") ? getFloat(params["maxLength"]) : 0.0f;
		}
	}

	void saveToFile(const std::string& filename)
	{
		std::ofstream ofs(filename);
		const auto key = [](const std::string& id) { return "\"" + id + "\": "; };
		ofs << "{" << std::endl;
		ofs << key("params"); m_params.toJSON(ofs); ofs << "," << std::endl;
		ofs << key("sceneId"); ofs << "\"" << m_sceneName << "\"," << std::endl;
		ofs << key("segIndices"); ofs << "[";
		for (unsigned int i = 0; i < m_segmentIds.size(); i++) {
			ofs << m_segmentIds[i];
			if (i + 1 < m_segmentIds.size()) ofs << ",";
		}
		ofs << "]" << std::endl;
		ofs << "}" << std::endl;
		ofs.close();
	}

	const std::vector<unsigned int>& getSegmentIdsPerVertex() const { return m_segmentIds; }
	size_t getNumSegments() const { return m_segIdsToVertIds.size(); }
	const std::unordered_map<unsigned int, std::vector<unsigned int>>& getSegmentIdToVertIdMap() const { return m_segIdsToVertIds; }

	bool empty() const { return m_segmentIds.empty(); }

	void computeSurfaceAreaPerSegment(const TriMeshf& triMesh, std::unordered_map<unsigned int, float>& segIdToSurfaceArea) const {
		segIdToSurfaceArea.clear();
		std::unordered_map<unsigned int, std::vector<unsigned int>> vertIdsToFaceIds;
		for (unsigned int i = 0; i < triMesh.m_indices.size(); i++) {
			const auto& ind = triMesh.m_indices[i];
			for (unsigned int k = 0; k < 3; k++) {
				auto it = vertIdsToFaceIds.find(ind[k]);
				if (it == vertIdsToFaceIds.end()) vertIdsToFaceIds[ind[k]] = std::vector<unsigned int>(1, i);
				else it->second.push_back(i);
			}
		}
		for (const auto& s : m_segIdsToVertIds) {
			std::unordered_map<unsigned int, unsigned int> faces; //<face id, counter>
			for (unsigned int vertid : s.second) {
				const auto itf = vertIdsToFaceIds.find(vertid);
				MLIB_ASSERT(itf != vertIdsToFaceIds.end());
				for (unsigned int fid : itf->second) {
					auto it = faces.find(fid);
					if (it == faces.end()) faces[fid] = 1;
					else it->second++;
				}
			}
			//compute sa
			float segmentSA = 0.0f;
			for (const auto& f : faces) {
				if (f.second == 3) {
					const auto& ind = triMesh.m_indices[f.first];
					Trianglef tri(triMesh.m_vertices[ind.x].position, triMesh.m_vertices[ind.y].position, triMesh.m_vertices[ind.z].position);
					segmentSA += tri.getArea();
				}
			}
			MLIB_ASSERT(segmentSA > 0.0f);
			segIdToSurfaceArea[s.first] = segmentSA;
		}
	}

private:
	void clear() {
		m_segmentIds.clear();
		m_sceneName = "";
		m_segIdsToVertIds.clear();
	}

	//TODO WTF IS THIS NECESSARY
	template<typename T>
	unsigned int getUINT(const rapidjson::GenericValue<T>& d) const {
		unsigned int res = (unsigned int)-1;
		if (d.IsInt()) res = (unsigned int)d.GetInt();
		else if (d.IsUint()) res = d.GetUint();
		else if (d.IsString()) res = util::convertTo<unsigned int>(d.GetString());
		else if (d.IsNull()) return res; //TODO HACK
		else {
			auto t = d.GetType();
			const bool b0 = d.IsDouble();
			const bool b1 = d.IsInt();
			const bool b2 = d.IsInt64();
			const bool b3 = d.IsUint();
			const bool b4 = d.IsUint64();
			throw MLIB_EXCEPTION("invalid json type for uint!");
		}
		return res;
	}
	template<typename T>
	float getFloat(const rapidjson::GenericValue<T>& d) const {
		float res = -std::numeric_limits<float>::infinity();
		if (d.IsDouble()) res = (float)d.GetDouble();
		else if (d.IsInt()) res = (float)d.GetInt();
		else if (d.IsString()) res = util::convertTo<float>(d.GetString());
		else throw MLIB_EXCEPTION("invalid json type for float!");
		return res;
	}

	std::vector<unsigned int> m_segmentIds; //correspond to vertices (indexed by vertex id)
	std::string m_sceneName;

	std::unordered_map<unsigned int, std::vector<unsigned int>> m_segIdsToVertIds;

	SegmentationParams m_params;
};
