#ifndef RESOURCES_JSON_SNIPPETS_HPP_
#define RESOURCES_JSON_SNIPPETS_HPP_

namespace resources {

const static char* mission_json_good_1 = R"(
{
    "BottleAssignments": [
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 1,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "A",
            "AlphanumericColor": "Red",
            "Shape": "Semicircle",
            "ShapeColor": "Brown",
            "Index": 2,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "P",
            "AlphanumericColor": "Orange",
            "Shape": "Pentagon",
            "ShapeColor": "Green",
            "Index": 3,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "9",
            "AlphanumericColor": "Green",
            "Shape": "Circle",
            "ShapeColor": "Purple",
            "Index": 4,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "W",
            "AlphanumericColor": "Orange",
            "Shape": "Triangle",
            "ShapeColor": "White",
            "Index": 5,
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
const static char* mission_json_bad_1 = R"(
{
    "BottleAssignments": [
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 1,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "A",
            "AlphanumericColor": "Red",
            "Shape": "Semicircle",
            "ShapeColor": "Brown",
            "Index": 2,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "P",
            "AlphanumericColor": "Orange",
            "Shape": "Pentagon",
            "ShapeColor": "Green",
            "Index": 3,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "9",
            "AlphanumericColor": "Green",
            "Shape": "Circle",
            "ShapeColor": "Purple",
            "Index": 4,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "W",
            "AlphanumericColor": "Orange",
            "Shape": "Triangle",
            "ShapeColor": "White",
            "Index": 5,
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

const static char* mission_json_2020 = R"(
{
    "BottleAssignments": [
        {
            "Alphanumeric": "",
            "AlphanumericColor": 0,
            "Shape": 0,
            "ShapeColor": 0,
            "Index": 1,
            "IsMannikin": true
        },
        {
            "Alphanumeric": "A",
            "AlphanumericColor": "Red",
            "Shape": "Semicircle",
            "ShapeColor": "Brown",
            "Index": 2,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "P",
            "AlphanumericColor": "Orange",
            "Shape": "Pentagon",
            "ShapeColor": "Green",
            "Index": 3,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "9",
            "AlphanumericColor": "Green",
            "Shape": "Circle",
            "ShapeColor": "Purple",
            "Index": 4,
            "IsMannikin": false
        },
        {
            "Alphanumeric": "W",
            "AlphanumericColor": "Orange",
            "Shape": "Triangle",
            "ShapeColor": "White",
            "Index": 5,
            "IsMannikin": false
        }
    ],
  "FlightBoundary": [
        {
          "Latitude": 38.1462694444444,
          "Longitude": -76.4281638888889
        },
        {
          "Latitude": 38.151625,
          "Longitude": -76.4286833333333
        },
        {
          "Latitude": 38.1518888888889,
          "Longitude": -76.4314666666667
        },
        {
          "Latitude": 38.1505944444444,
          "Longitude": -76.4353611111111
        },
        {
          "Latitude": 38.1475666666667,
          "Longitude": -76.4323416666667
        },
        {
          "Latitude": 38.1446666666667,
          "Longitude": -76.4329472222222
        },
        {
          "Latitude": 38.1432555555556,
          "Longitude": -76.4347666666667
        },
        {
          "Latitude": 38.1404638888889,
          "Longitude": -76.4326361111111
        },
        {
          "Latitude": 38.1407194444444,
          "Longitude": -76.4260138888889
        },
        {
          "Latitude": 38.1437611111111,
          "Longitude": -76.4212055555556
        },
        {
          "Latitude": 38.1473472222222,
          "Longitude": -76.4232111111111
        },
        {
          "Latitude": 38.1461305555556,
          "Longitude": -76.4266527777778
        }
    ],
  "AirdropBoundary": [
    {
      "Latitude": 38.1444444444444,
      "Longitude": -76.4280916666667
    },
    {
      "Latitude": 38.1459444444444,
      "Longitude": -76.4237944444445
    },
    {
      "Latitude": 38.1439305555556,
      "Longitude": -76.4227444444444
    },
    {
      "Latitude": 38.1417138888889,
      "Longitude": -76.4253805555556
    },
    {
      "Latitude": 38.1412111111111,
      "Longitude": -76.4322361111111
    },
    {
      "Latitude": 38.1431055555556,
      "Longitude": -76.4335972222222
    },
    {
      "Latitude": 38.1441805555556,
      "Longitude": -76.4320111111111
    },
    {
      "Latitude": 38.1452611111111,
      "Longitude": -76.4289194444444
    },
    {
      "Latitude": 38.1444444444444,
      "Longitude": -76.4280916666667
    }
  ],
  "Waypoints": [
    {
      "Latitude": 38.1446916666667,
      "Longitude": -76.4279944444445,
      "Altitude": 200.0
    },
    {
      "Latitude": 38.1461944444444,
      "Longitude": -76.4237138888889,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1438972222222,
      "Longitude": -76.42255,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.1417722222222,
      "Longitude": -76.4251083333333,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.14535,
      "Longitude": -76.428675,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1508972222222,
      "Longitude": -76.4292972222222,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1514944444444,
      "Longitude": -76.4313833333333,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1505333333333,
      "Longitude": -76.434175,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1479472222222,
      "Longitude": -76.4316055555556,
      "Altitude": 200.0
    },
    {
      "Latitude": 38.1443333333333,
      "Longitude": -76.4322888888889,
      "Altitude": 200.0
    },
    {
      "Latitude": 38.1433166666667,
      "Longitude": -76.4337111111111,
      "Altitude": 300.0
    },
    {
      "Latitude": 38.1410944444444,
      "Longitude": -76.4321555555556,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.1415777777778,
      "Longitude": -76.4252472222222,
      "Altitude": 400.0
    },
    {
      "Latitude": 38.1446083333333,
      "Longitude": -76.4282527777778,
      "Altitude": 200.0
    }
  ]
})";

}  // namespace resources

#endif  // RESOURCES_JSON_SNIPPETS_HPP_