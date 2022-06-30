#include "tests.h"

#include <vector>
#include <sstream>

#include "geo.h"
#include "graph.h"
// #include "input_reader.h"
#include "json_reader.h"
#include "transport_catalogue.h"

namespace tests {
    using namespace transport_catalogue;

    void TCAddStop() {
        TransportCatalogue tc;
        ASSERT(tc.GetStops().empty());
        ASSERT(tc.GetStopnames().empty());
        std::string stop_name = "Test1";
        Stop s {stop_name, 12.2, 76.8, 0};
        tc.AddStop(s);
        ASSERT_EQUAL(tc.GetStops().size(), 1);
        ASSERT_EQUAL(tc.GetStops()[0].name, "Test1");
        ASSERT_EQUAL(tc.GetStopnames()["Test1"]->name, "Test1");
    }

    void TCAddBus() {
        TransportCatalogue tc;
        ASSERT(tc.GetBuses().empty());
        ASSERT(tc.GetBusnames().empty());
        std::string stop_name1 = "Test1";
        std::string stop_name2 = "Test2";
        Stop s1 {stop_name1, 12.2, 76.8, 0};
        Stop s2 {stop_name2, 14.2, 77.2, 2};
        tc.AddStop(s1);
        tc.AddStop(s2);
        std::string bus_name = "Bus1";
        std::vector<Stop*> stops {tc.StopByName("Test1"), tc.StopByName("Test2")};
        tc.AddDistance(tc.StopByName("Test1"), tc.StopByName("Test2"), 25);
        Bus b {bus_name, stops, tc.GetDists(), true, tc.StopByName("Test1"), tc.StopByName("Test2")};
        tc.AddBus(b);
        ASSERT_EQUAL(tc.GetBuses().size(), 1);
        ASSERT_EQUAL(tc.GetBuses()[0].name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[0]->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[1]->name, "Test2");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->first->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->last->name, "Test2");
        ASSERT(tc.GetBusnames()["Bus1"]->is_roundtrip);
    }

    void InputAddStop() {
        TransportCatalogue tc;
        ASSERT(tc.GetStops().empty());
        ASSERT(tc.GetStopnames().empty());
        std::istringstream stream{R"({"type": "Stop","name": "Ривьерский мост","latitude": 43.587795,"longitude": 39.716901,"road_distances": {"Морской вокзал": 850}})"};
        const auto doc = json::Load(stream);
        json_reader::AddStop(doc.GetRoot().AsMap(), tc, 0);
        ASSERT_EQUAL(tc.GetStops().size(), 1);
        ASSERT_EQUAL(tc.GetStops()[0].name, "Ривьерский мост");
        ASSERT_EQUAL(tc.GetStopnames()["Ривьерский мост"]->name, "Ривьерский мост");
        ASSERT_APPOX_EQUAL(tc.GetStopnames()["Ривьерский мост"]->coords.lat, 43.587795);
        ASSERT_APPOX_EQUAL(tc.GetStopnames()["Ривьерский мост"]->coords.lng, 39.716901);
    }

    void InputAddDist() {
        TransportCatalogue tc;
        ASSERT(tc.GetBuses().empty());
        ASSERT(tc.GetBusnames().empty());
        ASSERT(tc.GetDists()->empty());
        std::string stop_name1 = "Test1";
        std::string stop_name2 = "Test2";
        Stop s1 {stop_name1, 12.2, 76.8, 0};
        Stop s2 {stop_name2, 14.2, 77.2, 2};
        tc.AddStop(s1);
        tc.AddStop(s2);

        std::istringstream stream{R"({"type": "Stop","name": "Test1","latitude": 43.587795,"longitude": 39.716901,"road_distances": {"Test2": 850}})"};
        const auto doc = json::Load(stream);
        graph::DirectedWeightedGraph<double> directed_graph(4);
        json_reader::RoutingSettings rs{6, 40};
        json_reader::AddDist(doc.GetRoot().AsMap(), tc, directed_graph, rs);

        ASSERT_EQUAL(tc.GetDists()->size(), 1);
        ASSERT_EQUAL(directed_graph.GetVertexCount(), 4);
        ASSERT_EQUAL(directed_graph.GetEdgeCount(), 1);
    }

    void InputAddBusOneWay() {
        TransportCatalogue tc;
        ASSERT(tc.GetBuses().empty());
        ASSERT(tc.GetBusnames().empty());
        std::string stop_name1 = "Test1";
        std::string stop_name2 = "Test2";
        Stop s1 {stop_name1, 12.201, 76.801, 0};
        Stop s2 {stop_name2, 12.202, 76.802, 2};
        tc.AddStop(s1);
        tc.AddStop(s2);
        tc.AddDistance(tc.StopByName("Test1"), tc.StopByName("Test2"), 200);

        json_reader::RoutingSettings rs{6, 40};
        graph::DirectedWeightedGraph<double> directed_graph(4);
        std::istringstream stream{R"({"type": "Bus","name": "Bus1","stops":["Test1", "Test2", "Test1"],"is_roundtrip": true})"};
        const auto doc = json::Load(stream);
        json_reader::AddBus(doc.GetRoot().AsMap(), tc, directed_graph, rs);

        ASSERT_EQUAL(tc.GetBuses().size(), 1);
        ASSERT_EQUAL(tc.GetBuses()[0].name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops.size(), 3);
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[0]->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[1]->name, "Test2");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[2]->name, "Test1");
        ASSERT_APPOX_EQUAL(tc.GetBusnames()["Bus1"]->GetCurvature(), 1.28628);
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->first->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->last->name, "Test1");
        ASSERT(tc.GetBusnames()["Bus1"]->is_roundtrip);
        ASSERT_EQUAL(directed_graph.GetEdgeCount(), 3);
    }

    void InputAddBusTwoWay() {
        TransportCatalogue tc;
        ASSERT(tc.GetBuses().empty());
        ASSERT(tc.GetBusnames().empty());
        std::string stop_name1 = "Test1";
        std::string stop_name2 = "Test2";
        Stop s1 {stop_name1, 12.201, 76.801, 0};
        Stop s2 {stop_name2, 12.202, 76.802, 2};
        tc.AddStop(s1);
        tc.AddStop(s2);
        tc.AddDistance(tc.StopByName("Test1"), tc.StopByName("Test2"), 200);

        json_reader::RoutingSettings rs{6, 40};
        graph::DirectedWeightedGraph<double> directed_graph(4);
        std::istringstream stream{R"({"type": "Bus","name": "Bus1","stops":["Test1", "Test2"],"is_roundtrip": false})"};
        const auto doc = json::Load(stream);
        json_reader::AddBus(doc.GetRoot().AsMap(), tc, directed_graph, rs);

        ASSERT_EQUAL(tc.GetBuses().size(), 1);
        ASSERT_EQUAL(tc.GetBuses()[0].name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops.size(), 3);
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[0]->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[1]->name, "Test2");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[2]->name, "Test1");
        ASSERT_APPOX_EQUAL(tc.GetBusnames()["Bus1"]->GetCurvature(), 1.28628);
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->first->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->last->name, "Test2");
        ASSERT(!tc.GetBusnames()["Bus1"]->is_roundtrip);
        ASSERT_EQUAL(directed_graph.GetEdgeCount(), 3);
    }

    void RunTests() {
        RUN_TEST(TCAddStop);
        RUN_TEST(TCAddBus);
        RUN_TEST(InputAddStop);
        RUN_TEST(InputAddDist);
        RUN_TEST(InputAddBusOneWay);
        RUN_TEST(InputAddBusTwoWay);
    }
}
