
#pragma once

#include "stdafx.h"

#include "processedFile.h"
#include "planeExtract.h"

class Alignment {
public:

	static MeshDataf makeNormalMesh(const vec3f& p0, const vec3f& p1, const vec4f& color = vec4f(0.8f, 0.2f, 0.2f, 1.0f))
	{
		TriMeshf normalMesh0 = Shapesf::sphere(0.05f, p0, 10, 10, color);
		TriMeshf normalMesh1 = Shapesf::cylinder(p0, p1, 0.025f, 10, 10, color);
		MeshDataf nMesh = normalMesh0.computeMeshData();
		nMesh.merge(normalMesh1.computeMeshData());
		return nMesh;
	}



	static vec3f upVectorFromViews(const SensorData& sd) {
		vec3f v(0.0f, 0.0f, 0.0f);
		for (size_t i = 0; i < sd.m_frames.size(); i++) {
			const mat4f& t = sd.m_frames[i].getCameraToWorld();
			if (sd.m_frames[i].getCameraToWorld()(0, 0) == -std::numeric_limits<float>::infinity()) continue;

			const vec3f cameraUp(0.0f, -1.0f, 0.0f);
			const vec3f worldUp = (t.getRotation() * cameraUp).getNormalized();
			v += worldUp;
		}
		v /= (float)sd.m_frames.size();
		return v.getNormalized();
	}

	static bool hasAccel(const SensorData& sd, unsigned int numThresh = 10) {
		unsigned int numValidAccel = 0;
		for (const SensorData::IMUFrame& f : sd.m_IMUFrames) {
			if (f.acceleration != vec3d(0.0)) numValidAccel++;
		}

		if (numValidAccel > numThresh) return true;
		else return false;
	}

	static vec3f upVectorFromAccel(const SensorData& sd)
	{
		if (sd.m_IMUFrames.size() == 0) throw MLIB_EXCEPTION("no imu data found");

		vec3f v(0.0f, 0.0f, 0.0f);
		for (size_t i = 0; i < sd.m_frames.size(); i++) {
			const SensorData::IMUFrame& f = sd.findClosestIMUFrame(i);
			
			if (sd.m_frames[i].getCameraToWorld()(0, 0) == -std::numeric_limits<float>::infinity()) continue;

			if (f.acceleration == vec3d::origin) {
				std::cout << "invalid IMU acceleration data entry at " << i << "-th frame" << std::endl;
				continue;
			}

			vec3f cameraUp = -vec3f((float)f.acceleration.x, (float)f.acceleration.y, (float)f.acceleration.z).getNormalized();
			const mat4f& t = sd.m_frames[i].getCameraToWorld();
			const vec3f worldUp = (t.getRotation() * cameraUp).getNormalized();
			v += worldUp;
		}

		return v.getNormalized();
	}

	static bool hasGravity(const SensorData& sd, unsigned int numThresh = 10)
	{
		unsigned int numValidGravity = 0;
		for (const SensorData::IMUFrame& f : sd.m_IMUFrames) {
			if (f.gravity != vec3d(0.0)) numValidGravity++;
		}

		if (numValidGravity > numThresh) return true;
		else return false;
	}

	static vec3f upVectorFromGravity(const SensorData& sd)
	{
		if (sd.m_IMUFrames.size() == 0) throw MLIB_EXCEPTION("no imu data found");

		vec3f v(0.0f, 0.0f, 0.0f);
		for (size_t i = 0; i < sd.m_frames.size(); i++) {
			const SensorData::IMUFrame& f = sd.findClosestIMUFrame(i);
			if (sd.m_frames[i].getCameraToWorld()(0, 0) == -std::numeric_limits<float>::infinity()) continue;

			if (f.gravity == vec3d::origin) {
				std::cout << "invalid IMU gravity data entry at " << i << "-th frame" << std::endl;
				continue;
			}

			vec3f cameraUp = vec3f((float)f.gravity.x, (float)f.gravity.y, (float)f.gravity.z).getNormalized();

			cameraUp = vec3f(cameraUp.y, cameraUp.x, cameraUp.z);

			const mat4f& t = sd.m_frames[i].getCameraToWorld();
			const vec3f worldUp = (t.getRotation() * cameraUp).getNormalized();
			v += worldUp;
		}
		v /= (float)sd.m_frames.size();

		return v.getNormalized();
	}

	static void writeTrajectoryFile(const std::string& outFile, const SensorData& sd) {

		std::vector<mat4f> trajecotry;
		for (size_t i = 0; i < sd.m_frames.size(); i++) {
			trajecotry.push_back(sd.m_frames[i].getCameraToWorld());
		}

		BinaryDataStreamFile outStream(outFile, true);
		outStream << trajecotry;
		outStream.close();
	}

	static std::vector<mat4f> readTrajectoryFile(const std::string& inFile) {
		BinaryDataStreamFile in(inFile, false);
		std::vector<mat4f> trajectory;
		in >> trajectory;
		return trajectory;
	}

	static void removeInvalidIMUFrames(SensorData& sd) {
		const bool checkTimeStamp = true;	//at the moment only remove invalid time frames
		const bool checkGravity = false;
		const bool checkAccel = false;

		unsigned int removedInvalidFrames = 0;
		for (std::vector<SensorData::IMUFrame>::iterator iter = sd.m_IMUFrames.begin(); iter != sd.m_IMUFrames.end();) {
			bool rem = false;
			if (checkTimeStamp && iter->timeStamp == 0) rem = true;
			if (checkGravity && iter->gravity == vec3d::origin) rem = true;
			if (checkAccel && iter->gravity == vec3d::origin) rem = true;

			if (rem == true) {
				iter = sd.m_IMUFrames.erase(iter);
				removedInvalidFrames++;
			}
			else {
				iter++;
			}
		}

		if (removedInvalidFrames > 0) {
			std::cout << "removed " << removedInvalidFrames << " invalid IMUFrames" << std::endl;
		}
	}

	static void alignScan(const std::string& path, bool forceRealign = false) {

		const std::string processedFile(path + "/" + "processed.txt");
		if (!util::fileExists(processedFile)) {
			std::cout << "no reconstruction available for " << path << "\n\t -> skipping folder" << std::endl;
			return;
		}
		ParameterFile parameterFile(processedFile);
		ProcessedFile pf;	pf.readMembers(parameterFile);
		if (!pf.valid) {
			std::cout << "reconstruction was invalid for " << path << "\n\t -> skipping folder" << std::endl;
			return;
		}
		if (pf.aligned && !forceRealign) {
			std::cout << "reconstruction is already aligned " << path << "\n\t -> skipping folder" << std::endl;
			return;
		}

		Directory dir(path);
		const std::vector<std::string> tmp = ml::util::split(util::replace(path, "\\", "/"), "/");	//we assume forward slashes
		const std::string base = tmp.back();
		const std::string baseFile = path + "/" + base;

		const std::string sensFile = path + "/" + base + ".sens";
		const std::string plyFile = path + "/" + base + ".ply";
		const std::string trajFile = util::replace(sensFile, ".sens", ".traj");


		mat4f transform = mat4f::identity();

		SensorData sd(sensFile);
		removeInvalidIMUFrames(sd);

		if (sd.m_frames.size() == 0) throw MLIB_EXCEPTION("no frames found in the sensor file");

		if (sd.m_frames[0].getCameraToWorld() != mat4f::identity()) {
			std::cout << "already found a previous alignment -> reverting to original" << std::endl;

			if (sd.m_frames[0].getCameraToWorld()(0, 0) == -std::numeric_limits<float>::infinity()) {
				std::cout << "error can't revert due to an invalid transform in the first frame" << std::endl;
				std::cout << "\tskipping folder " << std::endl;
				return;
			}
			mat4f inverse = sd.m_frames[0].getCameraToWorld().getInverse();
			sd.applyTransform(inverse);

			//if we have a multiple ply files (e.g., if VH was already run):
			Directory dir(path);
			std::vector<std::string> plyFiles = dir.getFilesWithSuffix(".ply");
			for (const std::string& plyFile : plyFiles) {
				MeshDataf md = MeshIOf::loadFromFile(dir.getPath() + "/" + plyFile);
				md.applyTransform(inverse);
				MeshIOf::saveToFile(dir.getPath() + "/" + plyFile, md);
			}
		}

		MeshDataf md = MeshIOf::loadFromFile(plyFile);
		md.mergeCloseVertices(0.0005f, true);
		md.removeIsolatedPieces(5000);

		//compute approx up vector from camera views or gravity (if available)
		if (true) {
			vec3f upEstimateView = upVectorFromViews(sd);
			vec3f upEstimate = upEstimateView;
			if (hasGravity(sd)) { //try to use gravity if possible
				vec3f upEstimateGrav = upVectorFromGravity(sd);
				upEstimate = upEstimateGrav;
			}

			vec3f x = upEstimate ^ vec3f(upEstimate.y, -upEstimate.z, upEstimate.x);	x.normalize();
			vec3f y = upEstimate ^ x;	y.normalize();
			vec3f z = upEstimate;
			mat4f mat = mat4f(x, y, z);
			md.applyTransform(mat);
			transform = mat * transform;
		}

		md.computeVertexNormals();
		
		//attempting to find a ground plane (and align to it)
		if (true) {
			PlaneExtract pe(md);
			pe.cluster();
			pe.removeSmallClusters();
			pe.removeNonBoundingClusters(0.1f, 100);	//the 
			const auto& clusters = pe.getClusters();

			bool foundHorizontalPlane = false;
			const size_t maxClusters = clusters.size();
			size_t i = 0;
			for (const Cluster& c : clusters) {
				if ((c.m_rep.plane.getNormal() | vec3f(0.0f, 0.0f, 1.0f)) > 0.8f) {
					mat4f mat = c.reComputeNormalAlignment();
					md.applyTransform(mat);
					transform = mat * transform;
					foundHorizontalPlane = true;
					break;
				}
				i++;
				if (i >= maxClusters) break;
			}
			if (!foundHorizontalPlane) std::cout << "could not find a horizontal plane" << std::endl;

			//final alignment with xy plane (translation)
			if (true) {
				BoundingBox3f bb = md.computeBoundingBox();
				mat4f mat = mat4f::translation(-bb.getMinZ());
				transform = mat * transform;
				md.applyTransform(mat);

				bb = md.computeBoundingBox();
				mat = mat4f::translation(-vec3f(bb.getCenter().x, bb.getCenter().y, 0.0f));
				md.applyTransform(mat);
				transform = mat * transform;
			}

			//attempting to find a vertical plane to align with the x and y axis
			if (true) {
				if (true) {
					//first find the bounding box using cgal
					OrientedBoundingBox3f obb = CGALWrapperf::computeOrientedBoundingBox(md.m_Vertices, CGALWrapperf::CONSTRAIN_Z);
					mat4f mat = mat4f(obb.getAxisX().getNormalized(), obb.getAxisY().getNormalized(), obb.getAxisZ().getNormalized());
					md.applyTransform(mat);
					transform = mat * transform;
				}

				//make sure x and y are in the positive quadrant
				if (true) {
					BoundingBox3f bb = md.computeBoundingBox();
					mat4f mat = mat4f::translation(-vec3f(bb.getMin().x, bb.getMin().y, 0.0f));
					md.applyTransform(mat);
					transform = mat * transform;
				}
			}
		}

		md.m_Normals.clear();

		{
			//if we have a multiple ply files (e.g., if VH was already run):
			Directory dir(path);
			std::vector<std::string> plyFiles = dir.getFilesWithSuffix(".ply");
			for (const std::string& plyFile : plyFiles) {
				MeshDataf mesh = MeshIOf::loadFromFile(dir.getPath() + "/" + plyFile);
				mesh.applyTransform(transform);
				MeshIOf::saveToFile(dir.getPath() + "/" + plyFile, mesh);
			}


			sd.applyTransform(transform);
			sd.saveToFile(sensFile);
			pf.aligned = true;	//it's now aligned
			pf.saveToFile(processedFile);
		}
	}

	static mat4f readTransformFromAln(const std::string& filename)
	{
		std::ifstream s(filename);
		if (!s.is_open()) throw MLIB_EXCEPTION("failed to open file " + filename + " for read");
		std::string tmp;
		std::getline(s, tmp); std::getline(s, tmp); std::getline(s, tmp);//ignore header lines
		mat4f m;
		for (unsigned int i = 0; i < 16; i++) s >> m[i];
		s.close();
		return m;
	}

	static void alignScanFromAlnFile(const std::string& path) {

		const std::string processedFile(path + "/" + "processed.txt");
		if (!util::fileExists(processedFile)) {
			std::cout << "no reconstruction available for " << path << "\n\t -> skipping folder" << std::endl;
			return;
		}
		ParameterFile parameterFile(processedFile);
		ProcessedFile pf;	pf.readMembers(parameterFile);
		if (!pf.valid) {
			std::cout << "reconstruction was invalid for " << path << "\n\t -> skipping folder" << std::endl;
			return;
		}

		Directory dir(path);
		const std::vector<std::string> tmp = ml::util::split(util::replace(path, "\\", "/"), "/");	//we assume forward slashes
		const std::string base = tmp.back();
		const std::string baseFile = path + "/" + base;

		const std::string alnFile = path + "/alignment.aln";
		const std::string sensFile = path + "/" + base + ".sens";
		std::vector<std::string> plyFiles = dir.getFilesWithSuffix(".ply");

		const mat4f transform = readTransformFromAln(alnFile);

		SensorData sd(sensFile);
		if (sd.m_frames.size() == 0) throw MLIB_EXCEPTION("no frames found in the sensor file");
		sd.applyTransform(transform);

		{ //save out transformed
			for (const std::string& plyFile : plyFiles) {
				MeshDataf mesh = MeshIOf::loadFromFile(dir.getPath() + "/" + plyFile);
				mesh.applyTransform(transform);
				MeshIOf::saveToFile(dir.getPath() + "/" + plyFile, mesh);
			}
			sd.applyTransform(transform);
			sd.saveToFile(sensFile);
			pf.aligned = true;	//it's now aligned
			pf.saveToFile(processedFile);
		}
	}


private:
};