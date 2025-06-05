#include <igl/copyleft/tetgen/tetrahedralize.h>
#include <igl/boundary_conditions.h>
#include <igl/readOBJ.h>
#include <igl/writeOBJ.h>
#include <igl/bbw.h>

#include <fstream>

#define OBJECTNAME "test.obj"
#define RIGNAME "rigArlo.rig"

int selected;

// Input polygon
Eigen::MatrixXd V;
Eigen::MatrixXi F;

// Tetrahedralized interior
Eigen::MatrixXd TV; 
Eigen::MatrixXi TT;
Eigen::MatrixXi TF;

// Handle de l'animation
Eigen::MatrixXd C;
Eigen::MatrixXi BE;
Eigen::MatrixXd W;

int main(int argc, char *argv[])
{
  using namespace Eigen;
  using namespace std;

  // Load a surface mesh
  igl::readOBJ(PROJECT_ROOT "objects/" OBJECTNAME,V,F);

  // Load rig
  std::ifstream myfile; myfile.open(PROJECT_ROOT "objects/" RIGNAME);
  if (!myfile.is_open()){
    cout << "unable to open rig file" << endl;
    exit(1);
  }
  
  string boneName;
  vector<string> boneNames;
  vector<vector<double>> head;
  vector<vector<double>> tail;
  while(myfile >> boneName){
    if (true){
    //if ((boneName[0] == 'D') && (boneName[1] == 'E') && (boneName[2] == 'F')){
      cout << ((boneName[0] == 'D') && (boneName[1] == 'E') && (boneName[2] == 'F')) << " " << boneName << endl;
      boneNames.push_back(boneName);
      float x,y,z;
      myfile >> x >> y >> z; head.push_back({x,z,-y});
      myfile >> x >> y >> z; tail.push_back({x,z,-y});
    }else{
      float d;
      myfile >> d >> d >> d >> d >> d >> d;
    }
    
  }

  /*  V.conservativeResize(V.rows()+boneNames.size()*2, V.cols());
  for(int i = 0; i < boneNames.size(); i++){
    V.row(V.rows()-2*i-1) = Eigen::Vector3d(head[i].data());
    V.row(V.rows()-2*i-2) = Eigen::Vector3d(tail[i].data());
  }*/

  // Tetrahedralize the interior
  igl::copyleft::tetgen::tetrahedralize(V,F,"pq1.414YO4a8e-5", TV,TT,TF);
  auto U = TV;

  // On choisit des points de controle

  C = Eigen::MatrixXd::Zero(2*boneNames.size(),3);
  for(int b = 0; b < boneNames.size(); b++){
    Eigen::Vector3d vhead(head[b].data());
    Eigen::Vector3d vtail(tail[b].data());

    auto middle = (vhead+vtail)/2;
    
    int ihead = 0; double dhead = 1e9;
    int itail = 0; double dtail = 1e9;

    for(int i = 0; i < TV.rows(); i++){
      Eigen::Vector3d vi = TV.row(i);
      double d = (vhead - vi).squaredNorm();
      if (d < dhead){
        dhead = d;
        ihead = i;
      }
      d = (vtail - vi).squaredNorm();
      if (d < dtail){
        dtail = d;
        itail = i;
      }
    }
    cout << boneNames[b] << " " << ihead << " " << itail << endl;
    cout << dhead << " " << dtail << " " << (vhead-vtail).squaredNorm() << endl;
    if (itail == ihead) cout << "--------------PROBLEM---------------" << endl;
    itail = 0; dtail = 1e9;
    for(int i = 0; i < TV.rows(); i++){
      if (i == ihead) continue;
      Eigen::Vector3d vi = TV.row(i);
      double d = (vtail - vi).squaredNorm();
      if (d < dtail){
        dtail = d;
        itail = i;
      }
    }
    cout << boneNames[b] << " " << ihead << " " << itail << endl;
    cout << dhead << " " << dtail << " " << (vhead-vtail).squaredNorm() << endl;

    C.row(2*b) = TV.row(ihead);
    C.row(2*b+1) = TV.row(itail);
  }
  
  BE = Eigen::MatrixXi::Zero(boneNames.size(),2);
  for(int i = 0; i < boneNames.size(); i++){
    BE(i,0) = 2*i;
    BE(i,1) = 2*i+1;
  }
  // List of boundary indices (aka fixed value indices into VV)
  VectorXi b;
  // List of boundary conditions of each weight function
  MatrixXd bc;
  bool result = igl::boundary_conditions(TV,TT,C,VectorXi(),BE,MatrixXi(),MatrixXi(),b,bc);
  cout << "RES : " << result << endl;

    // compute BBW weights matrix
  igl::BBWData bbw_data;
  // only a few iterations for sake of demo
  bbw_data.active_set_params.max_iter = 8;
  bbw_data.verbosity = 2;
  if(!igl::bbw(TV,TT,b,bc,bbw_data,W))
  {
    return EXIT_FAILURE;
  }

  // Normalize weights to sum to one
  W  = (W.array().colwise() / W.array().rowwise().sum()).eval();

  cout << "SIZE : " << TV.size() << " " << TT.size() << " " << W.size() << endl;
  
  // On enregistre les tets et leurs poids

  //igl::writeOBJ(PROJECT_ROOT "objects/tet" OBJECTNAME,TV,TT);

  // Export the result as an .obj file
	std::ofstream out;
	out.open(PROJECT_ROOT "objects/tet" OBJECTNAME);
	if (out.is_open() == false)
		return 0;
	for (size_t i = 0; i < TV.size()/3; i++)
		out << "v " << TV.row(i)[0] << " " << TV.row(i)[1] << " " << TV.row(i)[2] << '\n';
	for (size_t i = 0; i < TT.size()/4; i++)
    out << "t " << TT.row(i)[0] << " " << TT.row(i)[1] << " " << TT.row(i)[2] << " " << TT.row(i)[3] << '\n';
  for (size_t i = 0; i < W.size()/W.row(0).size(); i++){
    out << "vc ";
    for(size_t j = 0; j < W.row(0).size(); j++)
      out << W.row(i)[j] << " ";
    out << "\n";
  }

	out.close();

}
