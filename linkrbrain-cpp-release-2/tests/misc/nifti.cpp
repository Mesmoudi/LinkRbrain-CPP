#include "LinkRbrain/Parsing/NiftiParser.hpp"

typedef double T;
// static const std::string path = "data/default/Adult human brain/Datasets/areas/talairach.nii";
// static const std::string path = "./tmp/uploads/2013-10-22/sentences.dat_ri_z_fdr.nii.gz";
// static const std::string path = "data/mni_icbm152_nlin_asym_09b/mni_icbm152_t1_tal_nlin_asym_09b_hires.nii";
static const std::string path = "data/issues/test.nii.gz";


int main(int argc, char const *argv[]) {
    LinkRbrain::Parsing::NiftiParser parser;
    Types::Array3D<T> array3d;
    parser.parse(array3d, path);
    array3d.show(Types::Array3D<T>::X, 60, Types::Array3D<T>::Heatmap);
    return 0;
}
