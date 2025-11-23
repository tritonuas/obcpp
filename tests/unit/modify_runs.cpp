#include <gtest/gtest.h>
#include "cv/aggregator.hpp"
TEST(ModifyRuns, OverWriteData)
{
    PipelineParams params(std::nullopt,
                          0.35f,
                          1024,
                          1024,
                          "",
                          false);
    Pipeline pipeline(params);
    CVAggregator aggregator(std::move(pipeline));

    std::vector<IdentifiedTarget> updated;
    {
        auto lock = aggregator.getCVRecord();
        auto ptr = lock.data;
        // Add some mock runs
        for (int i = 1; i < 11; i++)
        {
            IdentifiedTarget target;
            target.set_run_id(i);
            // set mock bounding box
            GPSCoord *proto_coord = target.add_coordinates();
            proto_coord->set_latitude(0);
            proto_coord->set_longitude(0);
            proto_coord->set_altitude(0);

            BboxProto *proto_bbox = target.add_bboxes();
            proto_bbox->set_x1(0);
            proto_bbox->set_y1(0);
            proto_bbox->set_x2(0);
            proto_bbox->set_y2(0);

            ptr->insert_or_assign(i, target);
        }

        for (int i = 1; i < 11; i++)
        {
            IdentifiedTarget target;
            target.set_run_id(i);

            GPSCoord *proto_coord = target.add_coordinates(); // Use the plural field name
            proto_coord->set_latitude(i);
            proto_coord->set_longitude(i);
            proto_coord->set_altitude(i);

            // Add bounding box
            BboxProto *proto_bbox = target.add_bboxes(); // Use the plural field name
            proto_bbox->set_x1(i);
            proto_bbox->set_y1(i);
            proto_bbox->set_x2(i);
            proto_bbox->set_y2(i);
            ptr->insert_or_assign(i, target);

            updated.push_back(target);
        }
        // deallocate to unlock the mutex
    }
    aggregator.updateRecords(updated);

    for (auto value : updated)
    {
        auto lock = aggregator.getCVRecord();
        auto ptr = lock.data;
        ASSERT_TRUE(ptr->contains(value.run_id()));
        for (int i = 0; i < value.bboxes_size(); i++)
        {
            ASSERT_EQ(value.bboxes().at(i).x1(), ptr->at(value.run_id()).bboxes().at(i).x1());
            ASSERT_EQ(value.bboxes().at(i).y1(), ptr->at(value.run_id()).bboxes().at(i).y1());
            ASSERT_EQ(value.bboxes().at(i).x2(), ptr->at(value.run_id()).bboxes().at(i).x2());
            ASSERT_EQ(value.bboxes().at(i).y2(), ptr->at(value.run_id()).bboxes().at(i).y2());
        }
        for (int i = 0; i < value.coordinates_size(); i++)
        {
            ASSERT_EQ(value.coordinates().at(i).altitude(), ptr->at(value.run_id()).coordinates().at(i).altitude());
            ASSERT_EQ(value.coordinates().at(i).longitude(), ptr->at(value.run_id()).coordinates().at(i).longitude());
            ASSERT_EQ(value.coordinates().at(i).latitude(), ptr->at(value.run_id()).coordinates().at(i).latitude());
        }
    }
}