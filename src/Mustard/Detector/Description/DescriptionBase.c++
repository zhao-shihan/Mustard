#include "Mustard/Detector/Description/DescriptionBase.h++"
#include "Mustard/Detector/Description/DescriptionIO.h++"

namespace Mustard::Detector::Description {

DescriptionBase<>::DescriptionBase(std::string name) :
    NonMoveableBase{},
    fName{std::move(name)} {
    DescriptionIO::AddInstance(this);
}

auto DescriptionBase<>::Import(const YAML::Node& rootNode) -> void {
    const auto node{rootNode[fName]};
    if (node.IsDefined()) {
        ImportAllValue(node);
    } else {
        PrintNodeNotFoundWarning();
    }
}

auto DescriptionBase<>::Export(YAML::Node& rootNode) const -> void {
    auto node{rootNode[fName]};
    ExportAllValue(node);
}

} // namespace Mustard::Detector::Description
