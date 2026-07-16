#include "openbimrl/model/session.hpp"

#include <exception>
#include <filesystem>
#include <iostream>

#include <ifcparse/IfcFile.h>

namespace OpenBimRL::Native::Model {
namespace {

std::unique_ptr<IfcSession> g_active;
bool g_silent = false;

}  // namespace

IfcSession::IfcSession(std::unique_ptr<IfcParse::IfcFile> file)
    : file_(std::move(file)) {}

std::unique_ptr<IfcSession> IfcSession::open(std::string_view path) {
    if (path.empty()) {
        std::cerr << "no file given" << std::endl;
        return nullptr;
    }
    // IfcOpenShell's IfcFile ctor fseeks without null-checking fopen; refuse
    // missing paths here so initIfc returns false instead of SIGSEGV.
    std::error_code ec;
    if (!std::filesystem::is_regular_file(std::filesystem::path(path), ec)) {
        std::cerr << "Unable to open .ifc file: " << path << std::endl;
        return nullptr;
    }
    auto file = std::make_unique<IfcParse::IfcFile>(std::string(path));
    if (!file->good()) {
        std::cerr << "Unable to parse .ifc file" << std::endl;
        return nullptr;
    }
    return std::unique_ptr<IfcSession>(new IfcSession(std::move(file)));
}

SchemaKind IfcSession::schema() const {
    if (!file_) return SchemaKind::Unknown;
    const auto name = file_->schema()->name();
    if (name == "IFC2X3" || name == "IFC2x3") return SchemaKind::IFC2X3;
    if (name == "IFC4") return SchemaKind::IFC4;
    if (name.rfind("IFC4X3", 0) == 0) return SchemaKind::IFC4X3;
    return SchemaKind::Unknown;
}

std::vector<IfcEntityRef> IfcSession::instancesOf(
    std::string_view ifcClass) const {
    std::vector<IfcEntityRef> out;
    if (!file_ || ifcClass.empty()) return out;
    try {
        const auto instances = file_->instances_by_type(std::string(ifcClass));
        if (!instances) return out;
        out.reserve(instances->size());
        for (auto* item : *instances) {
            if (item) out.push_back(IfcEntityRef{item});
        }
    } catch (const IfcParse::IfcException&) {
        // Unknown / schema-mismatched type names must not kill the JVM.
        return {};
    } catch (const std::exception&) {
        return {};
    }
    return out;
}

IfcEntityRef IfcSession::instanceByGuid(std::string_view guid) const {
    if (!file_) return {};
    try {
        return IfcEntityRef{file_->instance_by_guid(std::string(guid))};
    } catch (const IfcParse::IfcException&) {
        return {};
    }
}

void ActiveSession::set(std::unique_ptr<IfcSession> session) {
    g_active = std::move(session);
}

IfcSession* ActiveSession::get() { return g_active.get(); }

IfcParse::IfcFile* ActiveSession::file() {
    return g_active ? g_active->file() : nullptr;
}

void ActiveSession::setSilent(bool silent) { g_silent = silent; }

bool ActiveSession::silent() { return g_silent; }

std::string getGuid(IfcEntityRef entity) {
    if (!entity) return {};
    const auto* base = static_cast<IfcUtil::IfcBaseClass*>(entity.ptr);
    const auto* asEntity = base->as<IfcUtil::IfcBaseEntity>(true);
    return asEntity->get_value<std::string>("GlobalId");
}

}  // namespace OpenBimRL::Native::Model
