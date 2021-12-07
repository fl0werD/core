/*
 *  Copyright (C) 2020 the_hunter
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAS_CSSDK_LIB
#include <core/rehlds_api.h>
#include <core/console.h>
#include <core/strings/consts.h>
#include <cssdk/engine/engine_hlds_api.h>
#include <cssdk/public/interface.h>

#ifdef HAS_AMXX_LIB
#include <amxx/config.h>
#else
#include <metamod/config.h>
#endif

using namespace cssdk;

namespace
{
#ifdef HAS_AMXX_LIB
    constexpr auto MODULE_NAME = amxx::MODULE_NAME;
#else
    constexpr auto MODULE_NAME = metamod::PLUGIN_NAME;
#endif
}

namespace core::rehlds_api
{
    bool Init()
    {
        using namespace detail;

        //
        // Load engine module.
        //

        auto* const engine_module = SysLoadModule(ENGINE_LIB);

        if (engine_module == nullptr) {
            console::Message<false>(str::EMPTY); // Line feed
            console::Error("Failed to locate engine module.\n");

            return false;
        }

        //
        // Get the engine interface factory.
        //

        const auto interface_factory = SysGetFactory(engine_module);

        if (interface_factory == nullptr) {
            console::Message<false>(str::EMPTY); // Line feed
            console::Error("Failed to locate interface factory in engine module.\n");

            return false;
        }

        //
        // Get the ReHLDS API interface.
        //

        auto ret_code = CreateInterfaceStatus::Failed;
        auto* const interface_base = interface_factory(VREHLDS_HLDS_API_VERSION, &ret_code);

        if (ret_code != CreateInterfaceStatus::Ok || interface_base == nullptr) {
            console::Message<false>(str::EMPTY); // Line feed
            console::Error("Failed to retrieve \"%s\" interface from engine module; return code %d.\n",
                           VREHLDS_HLDS_API_VERSION, static_cast<int>(ret_code));

            return false;
        }

        rehlds_api = reinterpret_cast<IReHldsApi*>(interface_base);

        //
        // Check the ReHLDS version.
        //

        const auto major_version = MajorVersion();
        const auto minor_version = MinorVersion();

        if (major_version != REHLDS_API_VERSION_MAJOR) {
            console::Message<false>(str::EMPTY); // Line feed
            console::Error("ReHLDS API major version mismatch; expected %d, real %d.",
                           REHLDS_API_VERSION_MAJOR, major_version);

            if (major_version < REHLDS_API_VERSION_MAJOR) {
                console::Message<false>("Please update the ReHLDS up to a major version API >= %d.\n",
                                        REHLDS_API_VERSION_MAJOR);
            }
            else {
                console::Message<false>("Please update the %s up to a major version API >= %d.\n",
                                        MODULE_NAME, REHLDS_API_VERSION_MAJOR);
            }

            return false;
        }

        if (minor_version < REHLDS_API_VERSION_MINOR) {
            console::Message<false>(str::EMPTY); // Line feed
            console::Error("ReHLDS API minor version mismatch; expected %d, real %d.",
                           REHLDS_API_VERSION_MINOR, minor_version);

            console::Message<false>("Please update the ReHLDS up to a minor version API >= %d.\n",
                                    REHLDS_API_VERSION_MINOR);

            return false;
        }

        //
        // RehldsApi assignment.
        //

        rehlds_funcs = rehlds_api->GetFuncs();
        assert(rehlds_funcs != nullptr);

        rehlds_hook_chains = rehlds_api->GetHookChains();
        assert(rehlds_hook_chains != nullptr);

        rehlds_server_static = rehlds_api->GetServerStatic();
        assert(rehlds_server_static != nullptr);

        rehlds_server_data = rehlds_api->GetServerData();
        assert(rehlds_server_data != nullptr);

        rehlds_flight_recorder = rehlds_api->GetFlightRecorder();
        assert(rehlds_flight_recorder != nullptr);

        return true;
    }
}
#endif
