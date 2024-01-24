// SPDX-License-Identifier: LGPL-2.1-or-later

// Tests for the makeShapeWithElementMap method, extracted from the main set of tests for TopoShape
// due to length and complexity.

#include "gtest/gtest.h"
#include "src/App/InitApplication.h"
#include "TopoShapeExpansionHelpers.h"
#include <Mod/Part/App/TopoShape.h>

#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_CompSolid.hxx>
#include <TopoDS_Compound.hxx>

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

TEST_F(TopoShapeMakeShapeWithElementMapTests, findShapeInElementMap)
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
