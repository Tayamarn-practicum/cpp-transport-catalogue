#include "tests.h"

#include <vector>

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

namespace tests {
    using namespace transport_catalogue;

    void TCAddStop() {
        TransportCatalogue tc;
        ASSERT(tc.GetStops().empty());
        ASSERT(tc.GetStopnames().empty());
        std::string stop_name = "Test1";
        Stop s {stop_name, 12.2, 76.8};
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
        Stop s1 {stop_name1, 12.2, 76.8};
        Stop s2 {stop_name2, 14.2, 77.2};
        tc.AddStop(s1);
        tc.AddStop(s2);
        std::string bus_name = "Bus1";
        std::vector<Stop*> stops {tc.StopByName("Test1"), tc.StopByName("Test2")};
        Bus b {bus_name, stops, tc};
        tc.AddBus(b);
        ASSERT_EQUAL(tc.GetBuses().size(), 1);
        ASSERT_EQUAL(tc.GetBuses()[0].name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[0]->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[1]->name, "Test2");
    }

    void InputAddStop() {
        TransportCatalogue tc;
        ASSERT(tc.GetStops().empty());
        ASSERT(tc.GetStopnames().empty());
        std::string req = "Stop Tolstopaltsevo: 55.611087, 37.208290";
        input_reader::AddStop(req, tc);
        ASSERT_EQUAL(tc.GetStops().size(), 1);
        ASSERT_EQUAL(tc.GetStops()[0].name, "Tolstopaltsevo");
        ASSERT_EQUAL(tc.GetStopnames()["Tolstopaltsevo"]->name, "Tolstopaltsevo");
        ASSERT_APPOX_EQUAL(tc.GetStopnames()["Tolstopaltsevo"]->coords.lat, 55.611087);
        ASSERT_APPOX_EQUAL(tc.GetStopnames()["Tolstopaltsevo"]->coords.lng, 37.208290);
    }

    void InputAddBusOneWay() {
        TransportCatalogue tc;
        ASSERT(tc.GetBuses().empty());
        ASSERT(tc.GetBusnames().empty());
        std::string stop_name1 = "Test1";
        std::string stop_name2 = "Test2";
        Stop s1 {stop_name1, 12.2, 76.8};
        Stop s2 {stop_name2, 14.2, 77.2};
        tc.AddStop(s1);
        tc.AddStop(s2);

        std::string req = "Bus Bus1: Test1 > Test2";
        input_reader::AddBus(req, tc);
        ASSERT_EQUAL(tc.GetBuses().size(), 1);
        ASSERT_EQUAL(tc.GetBuses()[0].name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops.size(), 2);
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[0]->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[1]->name, "Test2");
    }

    void InputAddBusTwoWay() {
        TransportCatalogue tc;
        ASSERT(tc.GetBuses().empty());
        ASSERT(tc.GetBusnames().empty());
        std::string stop_name1 = "Test1";
        std::string stop_name2 = "Test2";
        Stop s1 {stop_name1, 12.2, 76.8};
        Stop s2 {stop_name2, 14.2, 77.2};
        tc.AddStop(s1);
        tc.AddStop(s2);

        std::string req = "Bus Bus1: Test1 - Test2";
        input_reader::AddBus(req, tc);
        ASSERT_EQUAL(tc.GetBuses().size(), 1);
        ASSERT_EQUAL(tc.GetBuses()[0].name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->name, "Bus1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops.size(), 3);
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[0]->name, "Test1");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[1]->name, "Test2");
        ASSERT_EQUAL(tc.GetBusnames()["Bus1"]->stops[2]->name, "Test1");
    }

    void RunTests() {
        RUN_TEST(TCAddStop);
        RUN_TEST(TCAddBus);
        RUN_TEST(InputAddStop);
        RUN_TEST(InputAddBusOneWay);
        RUN_TEST(InputAddBusTwoWay);
    }
}
