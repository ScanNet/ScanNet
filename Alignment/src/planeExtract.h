
#pragma once

#include "stdafx.h"


struct Point {
	Planef plane;
	vec3f point;
	size_t urIdx;
};

struct Cluster {
	Cluster(const Point* init) {
		m_points.push_back(init);
		m_rep = *m_points.front();

		sumNormal = init->plane.getNormal();
		sumPoint = init->point;
	}
	void addPoint(const Point* p) {

		m_points.push_back(p);
		
		sumNormal += p->plane.getNormal();
		sumPoint += p->point;

		m_rep.plane = Planef(sumNormal.getNormalized(), sumPoint / (float)m_points.size());
	}

	bool check(const Point& p, float normalThresh, float distThresh) {
		float d_norm = p.plane.getNormal() | m_rep.plane.getNormal();
		float d_dist = m_rep.plane.distanceToPointAbs(p.point);

		return (d_norm > normalThresh) && (d_dist < distThresh);
	}

	mat4f reComputeNormalAlignment() const {
		std::vector<vec3f> points;
		for (const Point* p : m_points) {
			if (m_rep.plane.distanceToPointAbs(p->point) < 0.05f) {
				points.push_back(p->point);
			}
		}
		auto res = math::pointSetPCA(points);
		mat4f m(res[0].first.getNormalized(), res[1].first.getNormalized(), res[2].first.getNormalized());
		return m.getTranspose();
	}
	
	vec3f reComputeNormal() const {
		std::vector<vec3f> points;
		for (const Point* p : m_points) {
			if (m_rep.plane.distanceToPointAbs(p->point) < 0.05f) {
				points.push_back(p->point);
			}
		}
		auto res = math::pointSetPCA(points);
		mat3f m(res[0].first.getNormalized(), res[1].first.getNormalized(), res[2].first.getNormalized());
		return m.zcol().getNormalized();
	}

	bool operator<(const Cluster& other) const {
		return m_points.size() > other.m_points.size();
	}

	Point m_rep;
	std::list<const Point*> m_points;

	vec3f sumNormal;
	vec3f sumPoint;
};

class PlaneExtract {
public:
	PlaneExtract(const MeshDataf& md) {
		if (!md.hasNormals()) throw MLIB_EXCEPTION("need to compute normals first");

		m_points.resize(md.m_Vertices.size());
		for (size_t i = 0; i < md.m_Vertices.size(); i++) {
			Point& p = m_points[i];
			p.plane = Planef(md.m_Normals[i], md.m_Vertices[i]);
			p.point = md.m_Vertices[i];
			p.urIdx = i;
			m_pointsRef.push_back(&m_points[i]);
		}
	}

	void cluster(float normalThresh = 0.90f, float distThresh = 0.05f) {
		for (const Point* p : m_pointsRef) {
			bool foundCluster = false;
			for (Cluster& c : m_clusters) {
				if (c.check(*p, normalThresh, distThresh)) {
					c.addPoint(p);
					foundCluster = true;
					break;
				}
			}
			if (!foundCluster) {
				m_clusters.push_back(Cluster(p));
			}
		}

		m_clusters.sort();
	}

	void print(unsigned int topN = 10) const {
		std::cout << m_points.size() << " points " << std::endl;
		std::cout << m_clusters.size() << " clusters " << std::endl;
		unsigned int i = 0;
		for (const Cluster& c : m_clusters) {
			if (i >= topN) break;
			std::cout << "\t" << c.m_points.size() << std::endl;
			i++;
		}
	}


	const std::list<Cluster> getClusters() const {
		return m_clusters;
	}

	void removeSmallClusters(size_t minSize = 500) {
		for (std::list<Cluster>::iterator iter = m_clusters.begin(); iter != m_clusters.end();) {
			if (iter->m_points.size() < minSize) {
				iter = m_clusters.erase(iter);
			}
			else {
				iter++;
			}
		}
	}

	//threshold is 10cm by default
	void removeNonBoundingClusters(float distThresh, unsigned int numthresh) {
		for (std::list<Cluster>::iterator iter = m_clusters.begin(); iter != m_clusters.end();) {
			if (!isBoundingCluster(*iter, distThresh, numthresh)) {
				iter = m_clusters.erase(iter);
			}
			else {
				iter++;
			}
		}
	}

	//check whether there are points behind the plane -- if so then return false
	bool isBoundingCluster(const Cluster& c, float distThresh, unsigned int numThresh) const {
		unsigned int countBehind = 0;
		for (size_t i = 0; i < m_points.size(); i++) {
			const float d = c.m_rep.plane.distanceToPoint(m_points[i].point);
			if (d < -distThresh) countBehind++;
		}
		if (countBehind > numThresh) return false;
		else return true;
	}

	void saveColoredPlanes(const std::string& filename, const MeshDataf& md, unsigned int topN = 0) {
		topN = (unsigned int)std::min<size_t>(topN, m_clusters.size());

		MeshDataf mesh = md;

		if (mesh.m_Colors.size() != mesh.m_Vertices.size()) {
			mesh.m_Colors.resize(mesh.m_Vertices.size());
		}

		size_t i = 0;
		for (const Cluster& c : m_clusters) {
			RGBColor color = RGBColor::randomColor();
			for (const Point* p : c.m_points) {
				mesh.m_Colors[p->urIdx] = color;
			}

			i++;
			if (topN != 0 && i >= topN) {
				break;
			}
		}

		MeshIOf::saveToFile(filename, mesh);
	}


	void saveColoredPlane(const std::string& filename, const Cluster& c, const MeshDataf& md, const RGBColor& color = RGBColor::randomColor()) {

		MeshDataf mesh = md;

		if (mesh.m_Colors.size() != mesh.m_Vertices.size()) {
			mesh.m_Colors.resize(mesh.m_Vertices.size());
		}

		for (const Point* p : c.m_points) {
			mesh.m_Colors[p->urIdx] = color;
		}

		MeshIOf::saveToFile(filename, mesh);
	}

private:


	std::vector<Point> m_points;
	std::list<const Point*> m_pointsRef;
	std::list<Cluster> m_clusters;
};
