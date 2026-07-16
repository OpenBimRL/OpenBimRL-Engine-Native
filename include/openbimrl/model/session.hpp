#ifndef OPENBIMRL_NATIVE_MODEL_SESSION_H
#define OPENBIMRL_NATIVE_MODEL_SESSION_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <ifcparse/IfcFile.h>

#include "openbimrl/model/entity.hpp"

namespace OpenBimRL::Native::Model {

enum class SchemaKind { Unknown, IFC2X3, IFC4, IFC4X3 };

class IfcSession {
public:
    static std::unique_ptr<IfcSession> open(std::string_view path);

    IfcParse::IfcFile* file() const { return file_.get(); }
    SchemaKind schema() const;

    bool isIFC2x3() const { return schema() == SchemaKind::IFC2X3; }
    bool isIFC4() const { return schema() == SchemaKind::IFC4; }
    bool isIFC4X3() const { return schema() == SchemaKind::IFC4X3; }

    std::vector<IfcEntityRef> instancesOf(std::string_view ifcClass) const;
    IfcEntityRef instanceByGuid(std::string_view guid) const;

private:
    explicit IfcSession(std::unique_ptr<IfcParse::IfcFile> file);
    std::unique_ptr<IfcParse::IfcFile> file_;
};

/** Process-local active session (one model for JVM compatibility). */
class ActiveSession {
public:
    static void set(std::unique_ptr<IfcSession> session);
    static IfcSession* get();
    static IfcParse::IfcFile* file();
    static void setSilent(bool silent);
    static bool silent();

private:
    ActiveSession() = default;
};

std::string getGuid(IfcEntityRef entity);

}  // namespace OpenBimRL::Native::Model

#endif  // OPENBIMRL_NATIVE_MODEL_SESSION_H
