#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>

#include "tinyply.h"

using std::vector;
using std::string;

// felzenswalb segmentation (https://cs.brown.edu/~pff/segment/index.html)

// disjoint-set forests using union-by-rank and path compression (sort of).
typedef struct {
  int rank;
  int p;
  int size;
} uni_elt;

class universe {
 public:
  universe(int elements) {
    elts = new uni_elt[elements];
    num = elements;
    for (int i = 0; i < elements; i++) {
      elts[i].rank = 0;
      elts[i].size = 1;
      elts[i].p = i;
    }
  }
  ~universe() { delete [] elts; }
  int find(int x) {
    int y = x;
    while (y != elts[y].p)
      y = elts[y].p;
    elts[x].p = y;
    return y;
  }
  void join(int x, int y) {
    if (elts[x].rank > elts[y].rank) {
      elts[y].p = x;
      elts[x].size += elts[y].size;
    } else {
      elts[x].p = y;
      elts[y].size += elts[x].size;
      if (elts[x].rank == elts[y].rank)
        elts[y].rank++;
    }
    num--;
  }
  int size(int x) const { return elts[x].size; }
  int num_sets() const { return num; }
 private:
  uni_elt *elts;
  int num;
};

typedef struct {
  float w;
  int a, b;
} edge;

bool operator<(const edge &a, const edge &b) {
  return a.w < b.w;
}

universe *segment_graph(int num_vertices, int num_edges, edge *edges, float c) { 
  std::sort(edges, edges + num_edges);  // sort edges by weight
  universe *u = new universe(num_vertices);  // make a disjoint-set forest
  float *threshold = new float[num_vertices];
  for (int i = 0; i < num_vertices; i++) { threshold[i] = c; }
  // for each edge, in non-decreasing weight order
  for (int i = 0; i < num_edges; i++) {
    edge *pedge = &edges[i];
    // components conected by this edge
    int a = u->find(pedge->a);
    int b = u->find(pedge->b);
    if (a != b) {
      if ((pedge->w <= threshold[a]) && (pedge->w <= threshold[b])) {
        u->join(a, b);
        a = u->find(a);
        threshold[a] = pedge->w + (c / u->size(a));
      }
    }
  }
  delete [] threshold;
  return u;
}

// simple vec3f class
class vec3f {
 public:
  float x, y, z;
  vec3f() { x = 0; y = 0; z = 0; }
  vec3f(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
  vec3f operator+(const vec3f& o) {
    return vec3f{x+o.x, y+o.y, z+o.z};
  }
  vec3f operator-(const vec3f& o) {
    return vec3f{x-o.x, y-o.y, z-o.z};
  }
};
vec3f cross(const vec3f& u, const vec3f& v) {
  vec3f c = {u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x};
  float n = sqrtf(c.x*c.x + c.y*c.y + c.z*c.z);
  c.x /= n;  c.y /= n;  c.z /= n;
  return c;
}
vec3f lerp(const vec3f& a, const vec3f& b, const float v) {
  const float u = 1.0f-v;
  return vec3f(v*b.x + u*a.x, v*b.y + u*a.y, v*b.z + u*a.z);
}

vector<int> segment(const string& plyFile, const float kthr, const int segMinVerts) {
  // Load the geometry from .ply
  //std::cout << "Loading mesh " << plyFile << std::endl;
  std::ifstream ss(plyFile, std::ios::binary);
  tinyply::PlyFile file(ss);
  vector<float> verts;
  const size_t vertexCount = file.request_properties_from_element("vertex", { "x", "y", "z" }, verts);
  vector<uint32_t> faces;
  // Try getting vertex_indices or vertex_index
  size_t faceCount = file.request_properties_from_element("face", { "vertex_indices" }, faces, 3);
  if (faceCount == 0) {
    faceCount = file.request_properties_from_element("face", { "vertex_index" }, faces, 3);
  }
  file.read(ss);

  // create points, normals, edges, counts vectors
  vector<vec3f> points(vertexCount);
  vector<vec3f> normals(vertexCount);
  vector<int> counts(verts.size(), 0);
  const size_t edgesCount = faceCount*3;
  edge* edges = new edge[edgesCount];

  printf("Read ply with vertexCount %lu, faceCount %lu\n", vertexCount, faceCount);
  // Compute face normals and smooth into vertex normals
  for (int i = 0; i < faceCount; i++) {
    const int fbase = 3*i;
    const uint32_t i1 = faces[fbase];
    const uint32_t i2 = faces[fbase+1];
    const uint32_t i3 = faces[fbase+2];
    int vbase = 3*i1;
    vec3f p1(verts[vbase], verts[vbase+1], verts[vbase+2]);
    vbase = 3*i2;
    vec3f p2(verts[vbase], verts[vbase+1], verts[vbase+2]);
    vbase = 3*i3;
    vec3f p3(verts[vbase], verts[vbase+1], verts[vbase+2]);
    points[i1] = p1;  points[i2] = p2;  points[i3] = p3;
    const int ebase = 3*i;
    edges[ebase  ].a = i1;  edges[ebase  ].b = i2;
    edges[ebase+1].a = i1;  edges[ebase+1].b = i3;
    edges[ebase+2].a = i3;  edges[ebase+2].b = i2;

    // smoothly blend face normals into vertex normals
    vec3f normal = cross(p2 - p1, p3 - p1);
    normals[i1] = lerp(normals[i1], normal, 1.0f / (counts[i1] + 1.0f));
    normals[i2] = lerp(normals[i2], normal, 1.0f / (counts[i2] + 1.0f));
    normals[i3] = lerp(normals[i3], normal, 1.0f / (counts[i3] + 1.0f));
    counts[i1]++; counts[i2]++; counts[i3]++;
  }

  //std::cout << "Constructing edge graph based on mesh connectivity..." << std::endl;
  for (int i = 0; i < edgesCount; i++) {
    int a = edges[i].a;
    int b = edges[i].b;

    vec3f& n1 = normals[a];
    vec3f& n2 = normals[b];
    vec3f& p1 = points[a];
    vec3f& p2 = points[b];

    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    float dd = sqrtf(dx * dx + dy * dy + dz * dz); dx /= dd; dy /= dd; dz /= dd;
    float dot = n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
    float dot2 = n2.x * dx + n2.y * dy + n2.z * dz;
    float ww = 1.0f - dot;
    if (dot2 > 0) { ww = ww * ww; } // make it much less of a problem if convex regions have normal difference
    edges[i].w = ww;
  }
  //std::cout << "Constructed graph" << std::endl;

  // Segment!
  universe* u = segment_graph(vertexCount, edgesCount, edges, kthr);
  //std::cout << "Segmented" << std::endl;

  // Joining small segments
  for (int j = 0; j < edgesCount; j++) {
    int a = u->find(edges[j].a);
    int b = u->find(edges[j].b);
    if ((a != b) && ((u->size(a) < segMinVerts) || (u->size(b) < segMinVerts))) {
      u->join(a, b);
    }
  }

  // Return segment indices as vector
  vector<int> outComps(vertexCount);
  for (int q = 0; q < vertexCount; q++) {
    outComps[q] = u->find(q);
  }
  return outComps;
}

void writeToJSON(const string& filename, const string& scanId,
  const float kthr, const int segMinVerts, const vector<int>& segIndices) {
  std::ofstream ofs(filename);
  ofs << "{";
  ofs << "\"params\":{\"kThresh\":" << kthr <<  ",\"segMinVerts\":" << segMinVerts << "},";
  ofs << "\"sceneId\":\"" << scanId << "\",";
  ofs << "\"segIndices\":[";
  for (int i = 0; i < segIndices.size(); i++) {
    if (i > 0) { ofs << ","; }
    ofs << segIndices[i];
  }
  ofs << "]}";
  ofs.close();
}

int main(int argc, const char** argv) {
  if (argc < 2) {
    printf("Usage: ./segmentator input.ply [kThresh] [segMinVerts] (defaults: kThresh=0.01 segMinVerts=20)\n");
    exit(-1);
  } else {
    const string plyFile = argv[1];
    const float kthr = argc > 2 ? (float)atof(argv[2]) : 0.01f;
    const int segMinVerts = argc > 3 ? atoi(argv[3]) : 20;
    printf("Segmenting %s with kThresh=%f, segMinVerts=%d ...\n", plyFile.c_str(), kthr, segMinVerts);
    const vector<int> comps = segment(plyFile, kthr, segMinVerts);
    std::unordered_set<int> comp_indices;
    for (int i = 0; i < comps.size(); i++) {
      comp_indices.insert(comps[i]);
    }  
    const string baseName = plyFile.substr(0, plyFile.find_last_of("."));
    const int lastslash = plyFile.find_last_of("/");
    const string scanId = lastslash > 0 ? baseName.substr(lastslash) : baseName;
    string segFile = baseName + "." + std::to_string(kthr) + ".segs.json";
    writeToJSON(segFile, scanId, kthr, segMinVerts, comps);
    printf("Segmentation written to %s with %lu segments\n", segFile.c_str(), comp_indices.size());
  }
}
