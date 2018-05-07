#pragma once
#include "json.h"


class Aggregation {
	friend class Annotations;
public:
	Aggregation() {}
	Aggregation(const std::string& sceneId, const std::string& appId,
		const std::vector<std::unordered_set<unsigned int>>& aggregatedSegments,
		const std::vector<std::vector<std::string>>& aggregatedSegmentLabels, 
		const std::vector<unsigned int>& objectIds) :
		m_sceneId(sceneId), m_appId(appId),
		m_aggregatedSegments(aggregatedSegments),
		m_aggregatedSegmentLabels(aggregatedSegmentLabels),
		m_objectIds(objectIds) {}
	~Aggregation() {}

	bool empty() const { return m_aggregatedSegments.empty(); }
	size_t size() const { return m_aggregatedSegments.size(); }
	void clear() {
		m_sceneId = "";
		m_aggregatedSegments.clear();
		m_aggregatedAnnotationIds.clear();
		m_aggregatedSegmentLabels.clear();
	}
	bool empty(unsigned int i) const { return m_aggregatedSegments[i].empty(); }

	std::string getSceneId() const { return m_sceneId; }
	const std::vector<std::unordered_set<unsigned int>>& getAggregatedSegments() const { return m_aggregatedSegments; }
	const std::vector<unsigned int>& getObjectIds() const { return m_objectIds; }
	const std::unordered_map<unsigned int, std::string>& getObjectIdsToLabels() const { return m_objectIdsToLabels; }
	const std::vector<std::vector<std::string>>& getAggregatedSegmentLabels() const { return m_aggregatedSegmentLabels; }

	std::unordered_map<unsigned int, unsigned int> getSegIdToAggregatedSegId() const {
		std::unordered_map<unsigned int, unsigned int> mapping;
		for (unsigned int aggid = 0; aggid < m_aggregatedSegments.size(); aggid++) {
			const auto& agg = m_aggregatedSegments[aggid];
			for (unsigned int segid : agg) mapping[segid] = aggid;
		}
		return mapping;
	}

	void setSceneId(const std::string& sceneId) { m_sceneId = sceneId; }


	void loadFromJSONFile(const std::string& filename) {
		if (!util::fileExists(filename)) throw MLIB_EXCEPTION("failed to open file " + filename);
		rapidjson::Document aggregationData;
		if (!json::parseRapidJSONDocument(filename, &aggregationData)) {
			std::cerr << "Parse error reading " << filename << std::endl
				<< "Error code " << aggregationData.GetParseError() << " at " << aggregationData.GetErrorOffset() << std::endl;
			return;
		}
		//meta-info
		m_sceneId = aggregationData["sceneId"].GetString();
		m_appId = aggregationData["appId"].GetString();

		//segment groups
		const auto& segGroups = aggregationData["segGroups"];
		m_aggregatedSegments.resize(segGroups.Size());
		m_aggregatedSegmentLabels.resize(segGroups.Size());
		m_objectIds.resize(segGroups.Size());
		for (unsigned int i = 0; i < segGroups.Size(); i++) {
			const auto& group = segGroups[i];
			unsigned int id = getUINT(group["id"]);
			std::string label = group["label"].GetString();
			const auto& segs = group["segments"];
			std::vector<unsigned int> segments(segs.Size());
			for (unsigned int s = 0; s < segs.Size(); s++) {
				segments[s] = getUINT(segs[s]);
			}//segment ids (-> segmentation file)
			m_aggregatedSegments[i].insert(segments.begin(), segments.end());
			m_aggregatedSegmentLabels[i] = std::vector<std::string>(1, label);
			m_objectIdsToLabels[id] = label;
			m_objectIds[i] = id;
		}
	}
	void saveToJSONFile(const std::string& filename, const std::string& segFileSuffix = "_vh_clean_2.0.010000.segs.json") const {
		using namespace json;
		const bool endlines = true;

		std::ofstream ofs(filename);
		if (!ofs.good()) throw MLIB_EXCEPTION("failed to open " + filename + " for writing");
		const std::function<void(void)> sep = put(ofs, ",", endlines);
		const auto key = [](const std::string& id) { return "\"" + id + "\": "; };
		ofs << "{"; if (endlines) { ofs << std::endl; }
		ofs << key("sceneId"); toJSON(ofs, m_sceneId); sep();
		ofs << key("appId");   toJSON(ofs, m_appId);   sep();
		ofs << key("segGroups"); ofs << "["; if (endlines) { ofs << std::endl; }
		for (unsigned int i = 0; i < m_aggregatedSegments.size(); i++) { //each aggregated segment
			std::vector<unsigned int> aggregatedSegments(m_aggregatedSegments[i].begin(), m_aggregatedSegments[i].end()); //convert unordered_set to vector for json
			//write
			ofs << "\t{"; if (endlines) ofs << std::endl;
			ofs << "\t" << key("id"); toJSON(ofs, m_objectIds[i]); sep();
			ofs << "\t" << key("objectId");	toJSON(ofs, i); sep();
			ofs << "\t" << key("segments");	toJSON(ofs, aggregatedSegments); sep();
			ofs << "\t" << key("label");	toJSON(ofs, util::replace(m_aggregatedSegmentLabels[i].front(), "\\", "\\\\"));
			if (endlines) ofs << std::endl;
			ofs << "\t}"; 
			if (i + 1 < m_aggregatedSegments.size()) sep();
		}
		if (endlines) { ofs << std::endl; }
		ofs << "]"; sep(); //end segGroups
		ofs << key("segmentsFile"); toJSON(ofs, m_sceneId + segFileSuffix);
		ofs << "}"; if (endlines) { ofs << std::endl; }
		ofs.close();
	}
private:
	template<typename T>
	unsigned int getUINT(const rapidjson::GenericValue<T>& d) const {
		unsigned int res = (unsigned int)-1;
		if (d.IsInt()) res = (unsigned int)d.GetInt();
		else if (d.IsString()) res = util::convertTo<unsigned int>(d.GetString());
		else if (d.IsNull()) return res; //TODO HACK
		else throw MLIB_EXCEPTION("invalid json type for uint!");
		return res;
	}

	std::string m_sceneId;
	std::string m_appId;

	std::vector<std::unordered_set<unsigned int>> m_aggregatedSegments;	//vector of segment ids corresponding to an object
	std::vector<unsigned int>					  m_objectIds;			

	std::vector<std::vector<std::string>> m_aggregatedSegmentLabels;	//vector of text labels (e.g., "table", "microwave") corresponding to above

	//--not saved out in file below
	std::vector<std::vector<unsigned int>> m_aggregatedAnnotationIds;	//indexes to std::vector<Segment> m_annotations; 
	std::unordered_map<unsigned int, std::string> m_objectIdsToLabels;  
};

