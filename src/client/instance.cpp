#include "instance.hpp"

namespace selaura {
	instance& instance::get() {
		static selaura::instance inst;
		return inst;
	}

	void instance::start() {
		auto startTime = std::chrono::high_resolution_clock::now();
		selaura::io::init();

		selaura::detail::register_memory();
		selaura::init_hooking();
		this->hook_manager->for_each([&](auto& hook) {
			hook.enable();
		});

#ifdef SELAURA_WINDOWS
		winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, []() {
			winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().Title(winrt::to_hstring(std::format("Selaura Client {}{}", CLIENT_VERSION, DEVELOPER_MODE)));
		});
#endif

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> duration = endTime - startTime;

		selaura::io::info("Successfully injected [{:.2f}s]", duration.count());

		/*
		this->subscribe<test, &instance::func>();

		test ev( 1 );
		selaura::dispatcher<test>::dispatch(ev);
		*/
	}

	void instance::shutdown() {
		auto startTime = std::chrono::high_resolution_clock::now();
		selaura::io::info("Attempting to uninject");

		this->hook_manager->for_each([&](auto& hook) {
			hook.disable();
		});

#ifdef SELAURA_WINDOWS
		winrt::Windows::ApplicationModel::Core::CoreApplication::MainView().CoreWindow().Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, []() {
			winrt::Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().Title(winrt::to_hstring(""));
		});
#endif
		selaura::shutdown_hooking();

		auto endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> duration = endTime - startTime;

		selaura::io::info("Successfully uninjected [{:.2f}s]", duration.count());

#ifdef SELAURA_WINDOWS
		//FreeLibraryAndExitThread(selaura::hmodule, 0);
#endif

	}
};
