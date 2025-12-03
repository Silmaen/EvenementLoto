/**
 * @file Application.h
 * @author Silmaen
 * @date 02/12/2025
 * Copyright © 2025 All rights reserved.
 * All modification must get authorization from the author.
 */

#pragma once

namespace evl::gui_imgui {

/**
 * @brief Class Application.
 */
class Application {
public:
	/**
	 * @brief Default constructor.
	 */
	Application();
	/**
	 * @brief Default destructor.
	 */
	virtual ~Application();
	/**
	 * @brief Default copy constructor.
	 */
	Application(const Application&) = default;
	/**
	 * @brief Default move constructor.
	 */
	Application(Application&&) = default;
	/**
	 * @brief Default copy affectation operator.
	 */
	auto operator=(const Application&) -> Application& = default;
	/**
	 * @brief Default move affectation operator.
	 */
	auto operator=(Application&&) -> Application& = default;

	/**
	 * @brief Run the application.
	 */
	void run();

private:
};

auto createApplication(int iArgc, char* iArgv[]) -> std::shared_ptr<Application>;

}// namespace evl::gui_imgui
