#ifndef RESOURCES_JSON_SNIPPETS_HPP_
#define RESOURCES_JSON_SNIPPETS_HPP_

namespace resources {

const char* mission_json_good_1 = R"(
{
    "BottleAssignments": [
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 0,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "A",
            "AlphanumericColor": "Red",
            "Shape": "Semicircle",
            "ShapeColor": "Brown",
            "Index": 1,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "P",
            "AlphanumericColor": "Orange",
            "Shape": "Pentagon",
            "ShapeColor": "Green",
            "Index": 2,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "9",
            "AlphanumericColor": "Green",
            "Shape": "Circle",
            "ShapeColor": "Purple",
            "Index": 3,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "W",
            "AlphanumericColor": "Orange",
            "Shape": "Triangle",
            "ShapeColor": "White",
            "Index": 4,
            "IsMannikin": false
        }
    ],
    "FlightBoundary": [
        {
            "Latitude": 51.03027540382368,
            "Longitude": 9.952251183058683
        },
        {
            "Latitude": 50.98845393060358,
            "Longitude": 9.951220529522008
        },
        {
            "Latitude": 50.989751777263336,
            "Longitude": 10.043979347822926
        },
        {
            "Latitude": 51.02908205570247,
            "Longitude": 10.039513182497318
        }
    ],
    "AirdropBoundary": [
        {
            "Latitude": 51.021745818779976,
            "Longitude": 9.9653061278566
        },
        {
            "Latitude": 51.00055953825446,
            "Longitude": 9.965993230214371
        },
        {
            "Latitude": 51.000343931701856,
            "Longitude": 10.016151702332646
        },
        {
            "Latitude": 51.021305209993756,
            "Longitude": 10.01512104879597
        }
    ],
    "Waypoints": [
        {
            "Latitude": 51.006518228289636,
            "Longitude": 9.975440887633923,
            "Altitude": 75
        },
        {
            "Latitude": 51.00574668914667,
            "Longitude": 9.998115265440786,
            "Altitude": 75
        },
        {
            "Latitude": 51.01374278480851,
            "Longitude": 9.984029667106233,
            "Altitude": 75
        }
    ]
})";

// Bad because the flight boundary only has two coordinates
const char* mission_json_bad_1 = R"(
{
    "BottleAssignments": [
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 0,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "A",
            "AlphanumericColor": "Red",
            "Shape": "Semicircle",
            "ShapeColor": "Brown",
            "Index": 1,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "P",
            "AlphanumericColor": "Orange",
            "Shape": "Pentagon",
            "ShapeColor": "Green",
            "Index": 2,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "9",
            "AlphanumericColor": "Green",
            "Shape": "Circle",
            "ShapeColor": "Purple",
            "Index": 3,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "W",
            "AlphanumericColor": "Orange",
            "Shape": "Triangle",
            "ShapeColor": "White",
            "Index": 4,
            "IsMannikin": false
        }
    ],
    "FlightBoundary": [
        {
            "Latitude": 51.03027540382368,
            "Longitude": 9.952251183058683
        },
        {
            "Latitude": 50.98845393060358,
            "Longitude": 9.951220529522008
        }
    ],
    "AirdropBoundary": [
        {
            "Latitude": 51.021745818779976,
            "Longitude": 9.9653061278566
        },
        {
            "Latitude": 51.00055953825446,
            "Longitude": 9.965993230214371
        },
        {
            "Latitude": 51.000343931701856,
            "Longitude": 10.016151702332646
        },
        {
            "Latitude": 51.021305209993756,
            "Longitude": 10.01512104879597
        }
    ],
    "Waypoints": [
        {
            "Latitude": 51.006518228289636,
            "Longitude": 9.975440887633923,
            "Altitude": 75
        },
        {
            "Latitude": 51.00574668914667,
            "Longitude": 9.998115265440786,
            "Altitude": 75
        },
        {
            "Latitude": 51.01374278480851,
            "Longitude": 9.984029667106233,
            "Altitude": 75
        }
    ]
})";

}

#endif  // RESOURCES_JSON_SNIPPETS_HPP_