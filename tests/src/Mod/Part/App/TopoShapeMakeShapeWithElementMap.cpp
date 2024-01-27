// SPDX-License-Identifier: LGPL-2.1-or-later

// Tests for the makeShapeWithElementMap method, extracted from the main set of tests for TopoShape
// due to length and complexity.

#include "gtest/gtest.h"
#include "src/App/InitApplication.h"
#include "TopoShapeExpansionHelpers.h"
#include <Mod/Part/App/TopoShape.h>
#include <Mod/Part/App/TopoShapeOpCode.h>

#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Compound.hxx>

using namespace Part;
using namespace Data;

class TopoShapeMakeShapeWithElementMapTests: public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        tests::initApplication();
    }

    void SetUp() override
    {
        _docName = App::GetApplication().getUniqueDocumentName("test");
        App::GetApplication().newDocument(_docName.c_str(), "testUser");
        _sids = &_sid;
    }

    void TearDown() override
    {
        App::GetApplication().closeDocument(_docName.c_str());
    }

    Part::TopoShape* Shape()
    {
        return &_shape;
    }

    Part::TopoShape::Mapper* Mapper()
    {
        return &_mapper;
    }

private:
    std::string _docName;
    Data::ElementIDRefs _sid;
    QVector<App::StringIDRef>* _sids = nullptr;
    Part::TopoShape _shape;
    Part::TopoShape::Mapper _mapper;
};

TEST_F(TopoShapeMakeShapeWithElementMapTests, nullShapesThrows)
{
    // Arrange
    auto [cube1, cube2] = TopoShapeExpansionHelpers::CreateTwoCubes();
    std::vector<Part::TopoShape> sources {cube1, cube2};
    TopoDS_Vertex nullVertex;
    TopoDS_Edge nullEdge;
    TopoDS_Wire nullWire;
    TopoDS_Face nullFace;
    TopoDS_Shell nullShell;
    TopoDS_Solid nullSolid;
    TopoDS_CompSolid nullCompSolid;
    TopoDS_Compound nullCompound;

    // Act and assert
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullVertex, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullEdge, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullWire, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullFace, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullShell, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullSolid, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullCompSolid, *Mapper(), sources),
                 Part::NullShapeException);
    EXPECT_THROW(Shape()->makeShapeWithElementMap(nullCompound, *Mapper(), sources),
                 Part::NullShapeException);
}

TEST_F(TopoShapeMakeShapeWithElementMapTests, emptySourceShapes)
{
    // Arrange
    auto [cube1, cube2] = TopoShapeExpansionHelpers::CreateTwoCubes();
    std::vector<Part::TopoShape> emptySources;
    std::vector<Part::TopoShape> nonEmptySources {cube1, cube2};

    // Act and assert
    for (auto& source : nonEmptySources) {
        Part::TopoShape& modifiedShape = source;
        Part::TopoShape& originalShape = source;

        EXPECT_EQ(
            &originalShape,
            &modifiedShape.makeShapeWithElementMap(source.getShape(), *Mapper(), emptySources));
    }
}

TEST_F(TopoShapeMakeShapeWithElementMapTests, nonMappableSources)
{
    // Arrange
    auto [cube1, cube2] = TopoShapeExpansionHelpers::CreateTwoCubes();
    std::vector<Part::TopoShape> sources {cube1, cube2};

    // Act and assert
    for (auto& source : sources) {
        size_t canMap = 0;
        for (const auto& mappableSource : sources) {
            if (source.canMapElement(mappableSource)) {
                ++canMap;
            }
        }

        if (canMap == 0U) {
            EXPECT_EQ(&source,
                      &source.makeShapeWithElementMap(source.getShape(), *Mapper(), sources));
        }
    }
}

TEST_F(TopoShapeMakeShapeWithElementMapTests, findSourceShapesInShape)
{
    // Arrange
    auto [cube1, cube2] = TopoShapeExpansionHelpers::CreateTwoCubes();
    std::vector<Part::TopoShape> sources {cube1, cube2};
    sources[0].Tag = 1;  // setting Tag explicitly otherwise it is likely that this test will be
                         // more or less the same of nonMappableSources
    sources[1].Tag = 2;  // setting Tag explicitly otherwise it is likely that this test will be
                         // more or less the same of nonMappableSources
    Part::TopoShape cmpdShape;
    cmpdShape.makeElementCompound(sources);

    // Act and assert
    for (const auto& source : sources) {
        std::vector<Part::TopoShape> tmpSources {source};
        for (const auto& subSource : sources) {
            Part::TopoShape tmpShape {source.getShape()};
            tmpShape.makeShapeWithElementMap(source.getShape(), *Mapper(), tmpSources);
            if (&source == &subSource) {
                EXPECT_NE(tmpShape.findShape(subSource.getShape()),
                          0);  // if tmpShape uses, for example, cube1 and we search for cube1 than
                               // we should find it
            }
            else {
                EXPECT_EQ(tmpShape.findShape(subSource.getShape()),
                          0);  // if tmpShape uses, for example, cube1 and we search for cube2 than
                               // we shouldn't find it
            }
        }
        EXPECT_NE(cmpdShape.findShape(source.getShape()),
                  0);  // as cmpdShape is made with cube1 and cube2 we should find both of them
    }
}

TEST_F(TopoShapeMakeShapeWithElementMapTests, findSourceSubShapesInElementMap)
{
    // Arrange
    auto [cube1, cube2] = TopoShapeExpansionHelpers::CreateTwoCubes();
    std::vector<TopoShape> sources {cube1, cube2};
    sources[0].Tag = 1;  // setting Tag explicitly otherwise it is likely that this test will be
                         // more or less the same of nonMappableSources
    sources[1].Tag = 2;  // setting Tag explicitly otherwise it is likely that this test will be
                         // more or less the same of nonMappableSources

    // Act and assert
    // Testing with all the source TopoShapes
    for (const auto& source : sources) {
        TopoShape tmpShape {source.getShape()};
        tmpShape.makeShapeWithElementMap(source.getShape(), *Mapper(), sources);

        // First we create a map with the IndexedNames and MappedNames
        std::map<IndexedName, MappedName> elementStdMap;
        for (const auto& mappedElement : tmpShape.getElementMap()) {
            elementStdMap.emplace(mappedElement.index, mappedElement.name);
        }

        // Then for all the elements types (Vertex, Edge, Face) ...
        for (const auto& shapeType : source.getElementTypes()) {
            std::string shapeTypeStr {shapeType};

            // ... and all the elements of the various types in the source TopoShape ...
            for (unsigned long shapeIndex = 1U; shapeIndex <= source.countSubElements(shapeType);
                 shapeIndex++) {
                std::string shapeIndexStr = std::to_string(shapeIndex);
                std::string shapeName {shapeTypeStr + shapeIndexStr};

                IndexedName indexedName {shapeTypeStr.c_str(), (int)shapeIndex};
                MappedName mappedName {elementStdMap[indexedName]};
                const char shapeTypePrefix[1] {indexedName.toString()[0]};

                EXPECT_NO_THROW(elementStdMap.at(indexedName));  // .. we check that the IndexedName
                                                                 // is one of the keys...
                EXPECT_NE(mappedName.find(shapeName.c_str()),
                          -1);  // ... that the element name is in the MappedName...
                EXPECT_EQ(
                    mappedName.rfind(shapeTypePrefix),
                    mappedName.toString().length()
                        - 1);  // ... that the element prefix is at the end of the MappedName ...
            }

            // ... we also check that we don't find shapes that don't exist and therefor that don't
            // have neither an IndexedName nor a MappedName
            IndexedName fakeIndexedName {shapeTypeStr.c_str(),
                                         (int)source.countSubElements(shapeType) + 1};
            EXPECT_THROW(elementStdMap.at(fakeIndexedName), std::out_of_range);
        }
    }
}

TEST_F(TopoShapeMakeShapeWithElementMapTests, findMakerOpInElementMap)
{
    // Arrange
    auto [cube1, cube2] = TopoShapeExpansionHelpers::CreateTwoCubes();
    std::vector<TopoShape> sources {cube1, cube2};
    sources[0].Tag = 1;  // setting Tag explicitly otherwise it is likely that this test will be
                         // more or less the same of nonMappableSources
    sources[1].Tag = 2;  // setting Tag explicitly otherwise it is likely that this test will be
                         // more or less the same of nonMappableSources

    // Act and assert
    // Testing with all the source TopoShapes
    for (const auto& source : sources) {
        TopoShape tmpShape {source.getShape()};
        tmpShape.makeShapeWithElementMap(source.getShape(), *Mapper(), sources);

        // For all the mappedElements ...
        for (const auto& mappedElement : tmpShape.getElementMap()) {
            EXPECT_NE(mappedElement.name.find(OpCodes::Maker),
                      -1);  // ... we check that there's the "MAK" OpCode
        }
    }
}
