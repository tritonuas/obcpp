add_library(obcpp_pc_headers STATIC)

target_include_directories(obcpp_pc_headers PUBLIC ${INCLUDE_DIRECTORIES})

target_precompile_headers(obcpp_pc_headers 
    PUBLIC
        [["camera/interface.hpp"]]
        [["camera/lucid.hpp"]]
        [["camera/mock.hpp"]]

        [["core/mission_config.hpp"]]
        [["core/mission_state.hpp"]]
        [["core/obc.hpp"]]

        [["cv/aggregator.hpp"]]
        [["cv/classification.hpp"]]
        [["cv/hdr.hpp"]]
        [["cv/localization.hpp"]]
        [["cv/matching.hpp"]]
        [["cv/pipeline.hpp"]]
        [["cv/saliency.hpp"]]
        [["cv/segmentation.hpp"]]
        [["cv/utilities.hpp"]]

        [["network/airdrop_client.hpp"]]
        [["network/gcs_macros.hpp"]]
        [["network/gcs_routes.hpp"]]
        [["network/gcs.hpp"]]
        [["network/mavlink.hpp"]]
        [["network/server.hpp"]]

        [["pathing/airdrop.hpp"]]
        [["pathing/cartesian.hpp"]]
        [["pathing/dubins.hpp"]]
        [["pathing/dynamic.hpp"]]
        [["pathing/environment.hpp"]]
        [["pathing/plotting.hpp"]]
        [["pathing/static.hpp"]]
        [["pathing/tree.hpp"]]

        [["ticks/airdrop_approach.hpp"]]
        [["ticks/airdrop_prep.hpp"]]
        [["ticks/auto_landing.hpp"]]
        [["ticks/cv_loiter.hpp"]]
        [["ticks/fly_search.hpp"]]
        [["ticks/fly_waypoints.hpp"]]
        [["ticks/ids.hpp"]]
        [["ticks/manual_landing.hpp"]]
        [["ticks/mav_upload.hpp"]]
        [["ticks/mission_done.hpp"]]
        [["ticks/mission_prep.hpp"]]
        [["ticks/path_gen.hpp"]]
        [["ticks/path_validate.hpp"]]
        [["ticks/takeoff.hpp"]]
        [["ticks/tick.hpp"]]

        [["utilities/common.hpp"]]
        [["utilities/constants.hpp"]]
        [["utilities/datatypes.hpp"]]
        [["utilities/http.hpp"]]
        [["utilities/lockptr.hpp"]]
        [["utilities/locks.hpp"]]
        [["utilities/logging.hpp"]]
        [["utilities/obc_config.hpp"]]
        [["utilities/rng.hpp"]]
        [["utilities/serialize.hpp"]]
)