#include "macro-writer.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string.h"
//#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-io.h"

#include "../../source/core/slang-writer.h"

#include "../../source/compiler-core/slang-diagnostic-sink.h"
//#include "../../source/compiler-core/slang-name.h"

#include "diagnostics.h"
#include "options.h"
#include "node-tree.h"
#include "file-util.h"

namespace CppExtract
{
using namespace Slang;

SLANG_FORCE_INLINE static void _indent(Index indentCount, StringBuilder& out) { return FileUtil::indent(indentCount, out); }

SlangResult MacroWriter::calcDef(NodeTree* tree, SourceOrigin* origin, StringBuilder& out)
{
    for (Node* node : origin->m_nodes)
    {
        if (node->isReflected())
        {
            if (auto classLikeNode = as<ClassLikeNode>(node))
            {
                if (classLikeNode->m_marker.getContent().indexOf(UnownedStringSlice::fromLiteral("ABSTRACT")) >= 0)
                {
                    out << "ABSTRACT_";
                }

                out << "SYNTAX_CLASS(" << node->m_name.getContent() << ", " << classLikeNode->m_super.getContent() << ")\n";
                out << "END_SYNTAX_CLASS()\n\n";
            }
        }
    }
    return SLANG_OK;
}

SlangResult MacroWriter::calcChildrenHeader(NodeTree* tree, TypeSet* typeSet, StringBuilder& out)
{
    const List<ClassLikeNode*>& baseTypes = typeSet->m_baseTypes;
    const String& reflectTypeName = typeSet->m_typeName;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    List<ClassLikeNode*> classNodes;
    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        ClassLikeNode* baseType = baseTypes[i];        
        baseType->calcDerivedDepthFirst(classNodes);
    }

    //Node::filter(Node::isClassLike, nodes);

    List<ClassLikeNode*> derivedTypes;

    out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! CHILDREN !!!!!!!!!!!!!!!!!!!!!!!!!!!! */ \n\n";

    // Now the children 
    for (ClassLikeNode* classNode : classNodes)
    {
        classNode->getReflectedDerivedTypes(derivedTypes);

        // Define the derived types
        out << "#define " << m_options->m_markPrefix << "CHILDREN_" << reflectTypeName << "_"  << classNode->m_name.getContent() << "(x, param)";

        if (derivedTypes.getCount())
        {
            out << " \\\n";
            for (Index j = 0; j < derivedTypes.getCount(); ++j)
            {
                Node* derivedType = derivedTypes[j];
                _indent(1, out);
                out << m_options->m_markPrefix << "ALL_" << reflectTypeName << "_" << derivedType->m_name.getContent() << "(x, param)";
                if (j < derivedTypes.getCount() - 1)
                {
                    out << "\\\n";
                }
            }    
        }
        out << "\n\n";
    }

    out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! ALL !!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n\n";

    for (ClassLikeNode* classNode : classNodes)
    {
        // Define the derived types
        out << "#define " << m_options->m_markPrefix << "ALL_" << reflectTypeName << "_" << classNode->m_name.getContent() << "(x, param) \\\n";
        _indent(1, out);
        out << m_options->m_markPrefix << reflectTypeName << "_"  << classNode->m_name.getContent() << "(x, param)";

        // If has derived types output them
        if (classNode->hasReflectedDerivedType())
        {
            out << " \\\n";
            _indent(1, out);
            out << m_options->m_markPrefix << "CHILDREN_" << reflectTypeName << "_" << classNode->m_name.getContent() << "(x, param)";
        }
        out << "\n\n";
    }

    if (m_options->m_outputFields)
    {
        out << "\n\n /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!! FIELDS !!!!!!!!!!!!!!!!!!!!!!!!!!!! */\n\n";

        for (ClassLikeNode* classNode : classNodes)
        {
            // Define the derived types
            out << "#define " << m_options->m_markPrefix << "FIELDS_" << reflectTypeName << "_" << classNode->m_name.getContent() << "(_x_, _param_)";

            // Find all of the instance fields fields
            List<FieldNode*> fields;
            for (Node* child : classNode->m_children)
            {
                if (auto field = as<FieldNode>(child))
                {
                    if (!field->m_isStatic)
                    {
                        fields.add(field);
                    }
                }
            }

            if (fields.getCount() > 0)
            {
                out << "\\\n";

                const Index fieldsCount = fields.getCount();
                bool previousField = false;
                for (Index j = 0; j < fieldsCount; ++j) 
                {
                    const FieldNode* field = fields[j];
                        
                    if (field->isReflected())
                    {
                        if (previousField)
                        {
                            out << "\\\n";
                        }

                        _indent(1, out);

                        // NOTE! We put the type field in brackets, such that there is no issue with templates containing a comma.
                        // If stringified
                        out << "_x_(" << field->m_name.getContent() << ", (" << field->m_fieldType << "), _param_)";
                        previousField = true;
                    }
                }
            }
                
            out << "\n\n";
        }
    }

    return SLANG_OK;
}

SlangResult MacroWriter::calcOriginHeader(NodeTree* tree, StringBuilder& out)
{
    // Do macros by origin

    out << "// Origin macros\n\n";

    for (SourceOrigin* origin : tree->getSourceOrigins())
    {   
        out << "#define " << m_options->m_markPrefix << "ORIGIN_" << origin->m_macroOrigin << "(x, param) \\\n";

        for (Node* node : origin->m_nodes)
        {
            if (!(node->isReflected() && node->isClassLike()))
            {
                continue;
            }

            _indent(1, out);
            out << "x(" << node->m_name.getContent() << ", param) \\\n";
        }
        out << "/* */\n\n";
    }

    return SLANG_OK;
}

SlangResult MacroWriter::calcTypeHeader(NodeTree* tree, TypeSet* typeSet, StringBuilder& out)
{
    const List<ClassLikeNode*>& baseTypes = typeSet->m_baseTypes;
    const String& reflectTypeName = typeSet->m_typeName;

    out << "#pragma once\n\n";
    out << "// Do not edit this file is generated from slang-cpp-extractor tool\n\n";

    if (baseTypes.getCount() == 0)
    {
        return SLANG_OK;
    }

    // Set up the scope
    List<Node*> baseScopePath;
    baseTypes[0]->calcScopePath(baseScopePath);

    // Remove the global scope
    baseScopePath.removeAt(0);
    // Remove the type itself
    baseScopePath.removeLast();

    for (Node* scopeNode : baseScopePath)
    {
        SLANG_ASSERT(scopeNode->m_kind == Node::Kind::Namespace);
        out << "namespace " << scopeNode->m_name.getContent() << " {\n";
    }

    // Add all the base types, with in order traversals
    List<ClassLikeNode*> nodes;
    for (Index i = 0; i < baseTypes.getCount(); ++i)
    {
        ClassLikeNode* baseType = baseTypes[i];
        baseType->calcDerivedDepthFirst(nodes);
    }

    Node::filter(Node::isClassLikeAndReflected, nodes);

    // Write out the types
    {
        out << "\n";
        out << "enum class " << reflectTypeName << "Type\n";
        out << "{\n";

        Index typeIndex = 0;
        for (ClassLikeNode* node : nodes)
        {
            // Okay first we are going to output the enum values
            const Index depth = node->calcDerivedDepth() - 1;
            _indent(depth, out);
            out << node->m_name.getContent() << " = " << typeIndex << ",\n";
            typeIndex++;
        }

        _indent(1, out);
        out << "CountOf\n";

        out << "};\n\n";
    }

    // TODO(JS):
    // Strictly speaking if we wanted the types to be in different scopes, we would have to
    // change the namespaces here

    // Predeclare the classes
    {
        out << "// Predeclare\n\n";
        for (ClassLikeNode* node : nodes)
        {
            // If it's not reflected we don't output, in the enum list
            if (node->isReflected())
            {
                const char* type = (node->m_kind == Node::Kind::ClassType) ? "class" : "struct";
                out << type << " " << node->m_name.getContent() << ";\n";
            }
        }
    }

    // Do the macros for each of the types

    {
        out << "// Type macros\n\n";

        out << "// Order is (NAME, SUPER, ORIGIN, LAST, MARKER, TYPE, param) \n";
        out << "// NAME - is the class name\n";
        out << "// SUPER - is the super class name (or NO_SUPER)\n";
        out << "// ORIGIN - where the definition was found\n";
        out << "// LAST - is the class name for the last in the range (or NO_LAST)\n";
        out << "// MARKER - is the text inbetween in the prefix/postix (like ABSTRACT). If no inbetween text is is 'NONE'\n";
        out << "// TYPE - Can be BASE, INNER or LEAF for the overall base class, an INNER class, or a LEAF class\n";
        out << "// param is a user defined parameter that can be parsed to the invoked x macro\n\n";

        // Output all of the definitions for each type
        for (ClassLikeNode* node : nodes)
        {
            out << "#define " << m_options->m_markPrefix <<  reflectTypeName << "_" << node->m_name.getContent() << "(x, param) ";

            // Output the X macro part
            _indent(1, out);
            out << "x(" << node->m_name.getContent() << ", ";

            if (node->m_superNode)
            {
                out << node->m_superNode->m_name.getContent() << ", ";
            }
            else
            {
                out << "NO_SUPER, ";
            }

            // Output the (file origin)
            out << node->m_origin->m_macroOrigin;
            out << ", ";

            // The last type
            Node* lastDerived = node->findLastDerived();
            if (lastDerived)
            {
                out << lastDerived->m_name.getContent() << ", ";
            }
            else
            {
                out << "NO_LAST, ";
            }

            // Output any specifics of the markup
            UnownedStringSlice marker = node->m_marker.getContent();
            // Need to extract the name
            if (marker.getLength() > m_options->m_markPrefix.getLength() + m_options->m_markSuffix.getLength())
            {
                marker = UnownedStringSlice(marker.begin() + m_options->m_markPrefix.getLength(), marker.end() - m_options->m_markSuffix.getLength());
            }
            else
            {
                marker = UnownedStringSlice::fromLiteral("NONE");
            }
            out << marker << ", ";

            if (node->m_superNode == nullptr)
            {
                out << "BASE, ";
            }
            else if (node->hasReflectedDerivedType())
            {
                out << "INNER, ";
            }
            else
            {
                out << "LEAF, ";
            }
            out << "param)\n";
        }
    }

    // Now pop the scope in revers
    for (Index j = baseScopePath.getCount() - 1; j >= 0; j--)
    {
        Node* scopeNode = baseScopePath[j];
        out << "} // namespace " << scopeNode->m_name.getContent() << "\n";
    }

    return SLANG_OK;
}

SlangResult MacroWriter::writeDefs(NodeTree* tree)
{
    const auto& origins = tree->getSourceOrigins();

    for (SourceOrigin* origin : origins)
    {
        const String path = origin->m_sourceFile->getPathInfo().foundPath;

        // We need to work out the name of the def file

        String ext = Path::getPathExt(path);
        String pathWithoutExt = Path::getPathWithoutExt(path);

        // The output path

        StringBuilder outPath;
        outPath << pathWithoutExt << "-defs." << ext;

        StringBuilder content;
        SLANG_RETURN_ON_FAIL(calcDef(tree, origin, content));

        // Write the defs file
        SLANG_RETURN_ON_FAIL(FileUtil::writeAllText(outPath, m_sink, content.getUnownedSlice()));
    }

    return SLANG_OK;
}

SlangResult MacroWriter::writeOutput(NodeTree* tree)
{
    String path;
    if (m_options->m_inputDirectory.getLength())
    {
        path = Path::combine(m_options->m_inputDirectory, m_options->m_outputPath);
    }
    else
    {
        path = m_options->m_outputPath;
    }

    // Get the ext
    String ext = Path::getPathExt(path);
    if (ext.getLength() == 0)
    {
        // Default to .h if not specified
        ext = "h";
    }

    // Strip the extension if set
    path = Path::getPathWithoutExt(path);

    for (TypeSet* typeSet : tree->getTypeSets())
    {
        {
            /// Calculate the header
            StringBuilder header;
            SLANG_RETURN_ON_FAIL(calcTypeHeader(tree, typeSet, header));

            // Write it out

            StringBuilder headerPath;
            headerPath << path << "-" << typeSet->m_fileMark << "." << ext;
            SLANG_RETURN_ON_FAIL(FileUtil::writeAllText(headerPath, m_sink, header.getUnownedSlice()));
        }

        {
            StringBuilder childrenHeader;
            SLANG_RETURN_ON_FAIL(calcChildrenHeader(tree, typeSet, childrenHeader));

            StringBuilder headerPath;
            headerPath << path << "-" << typeSet->m_fileMark << "-macro." + ext;
            SLANG_RETURN_ON_FAIL(FileUtil::writeAllText(headerPath, m_sink, childrenHeader.getUnownedSlice()));
        }
    }

    return SLANG_OK;
}

} // namespace CppExtract
