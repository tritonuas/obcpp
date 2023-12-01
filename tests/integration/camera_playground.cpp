// Trigger software used for acquiring a single image from the Lucid Camera
// Code sourced from Lucid, settings adjusted by Anthony & Miles

#include "stdafx.h"
#include "GenTL.h"
#include "SaveApi.h"
#include <chrono>

#include <iostream>
#include <fstream>
#include <filesystem>

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#include "GenICam.h"

#ifdef __linux__
#pragma GCC diagnostic pop
#endif

#include "ArenaApi.h"

#define TAB1 "  "
#define TAB2 "    "
#define TAB3 "      "

#define PIXEL_FORMAT BGR8

// Basic trigger configuration and use. In order to configure trigger, enable
// trigger mode and set the source and selector. The trigger must be armed
// before it is prepared to execute. Once the trigger is armed, execute the
// trigger and retrieve an image.

// image timeout
#define TIMEOUT 2000

void SaveImage(Arena::IImage *pImage, const char *filename)
{

	// Convert image
	//    Convert the image to a displayable pixel format.
	// 	  Converts the image so that it is displayable by the operating system.
	auto pConverted = Arena::ImageFactory::Convert(
		pImage,
		PIXEL_FORMAT);

	// Prepare image parameters
	//    An image's width, height, and bits per pixel are required to save to
	//    disk. Its size and stride (i.e. pitch) can be calculated from those 3
	//    inputs. Notice that an image's size and stride use bytes as a unit
	//    while the bits per pixel uses bits.

	Save::ImageParams params(
		pConverted->GetWidth(),
		pConverted->GetHeight(),
		pConverted->GetBitsPerPixel());

	// Prepare image writer
	//    The image writer requires 3 arguments to save an image: the image's
	//    parameters, a specified file name or pattern, and the image data to
	//    save. Providing these should result in a successfully saved file on the
	//    disk. Because an image's parameters and file name pattern may repeat,
	//    they can be passed into the image writer's constructor.

	Save::ImageWriter writer(
		params,
		filename);

	// Save image
	//    Passing image data into the image writer using the cascading I/O
	//    operator (<<) triggers a save. Notice that the << operator accepts the
	//    image data as a constant unsigned 8-bit integer pointer (const
	//    uint8_t*) and the file name as a character string (const char*).
	writer << pConverted->GetData();

	// destroy converted image
	Arena::ImageFactory::Destroy(pConverted);
}

// demonstrates basic trigger configuration and use
// (1) sets trigger mode, source, and selector
// (2) starts stream
// (3) waits until trigger is armed
// (4) triggers image
// (5) gets image
// (6) requeues buffer
// (7) stops stream
void ConfigureTriggerAndAcquireImage(Arena::IDevice *pDevice)
{
	// get node values that will be changed in order to return their values at
	// the end of the example
	GenICam::gcstring triggerSelectorInitial = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "TriggerSelector");
	GenICam::gcstring triggerModeInitial = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "TriggerMode");
	GenICam::gcstring triggerSourceInitial = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "TriggerSource");

	// Set trigger selector
	//    Set the trigger selector to FrameStart. When triggered, the device will
	//    start acquiring a single frame. This can also be set to
	//    AcquisitionStart or FrameBurstStart.

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"TriggerSelector",
		"FrameStart");

	// Set trigger mode
	//    Enable trigger mode before setting the source and selector and before
	//    starting the stream. Trigger mode cannot be turned on and off while the
	//    device is streaming.

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"TriggerMode",
		"On");

	// Set trigger source
	//    Set the trigger source to software in order to trigger images without
	//    the use of any additional hardware. Lines of the GPIO can also be used
	//    to trigger.
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"TriggerSource",
		"Software");

	// enable  rolling shutter
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"SensorShutterMode",
		"Rolling");

	// enable auto exposure
	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"ExposureAuto",
		"Continuous");

	// turn on acquisition frame rate & set to max value
	Arena::SetNodeValue<bool>(
		pDevice->GetNodeMap(),
		"AcquisitionFrameRateEnable",
		true);
	GenApi::CFloatPtr pAcquisitionFrameRate = pDevice->GetNodeMap()->GetNode("AcquisitionFrameRate");
	double acquisitionFrameRate = pAcquisitionFrameRate->GetMax();
	pAcquisitionFrameRate->SetValue(acquisitionFrameRate);

	// Set target brightness to 70, suggested value from Lucid for outdoor imaging
	Arena::SetNodeValue<int64_t>(
		pDevice->GetNodeMap(),
		"TargetBrightness",
		70);

	Arena::SetNodeValue<bool>(
		pDevice->GetNodeMap(),
		"GammaEnable",
		true);

	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"Gamma",
		0.5);

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"GainAuto",
		"Continuous");

	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"ExposureAutoDamping",
		1);

	Arena::SetNodeValue<GenICam::gcstring>(
		pDevice->GetNodeMap(),
		"ExposureAutoAlgorithm",
		"Median");

	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"ExposureAutoUpperLimit",
		500);

	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"ExposureAutoLowerLimit",
		360);

	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"GainAutoUpperLimit",
		10);
	Arena::SetNodeValue<double>(
		pDevice->GetNodeMap(),
		"GainAutoLowerLimit",
		1);

	// enable stream auto negotiate packet size
	Arena::SetNodeValue<bool>(
		pDevice->GetTLStreamNodeMap(),
		"StreamAutoNegotiatePacketSize",
		true);

	// enable stream packet resend
	Arena::SetNodeValue<bool>(
		pDevice->GetTLStreamNodeMap(),
		"StreamPacketResendEnable",
		true);

	// Start stream
	//    When trigger mode is off and the acquisition mode is set to stream
	//    continuously, starting the stream will have the camera begin acquiring
	//    a steady stream of images. However, with trigger mode enabled, the
	//    device will wait for the trigger before acquiring any.

	pDevice->StartStream();

	const auto p1 = std::chrono::system_clock::now();
	const auto time = std::chrono::duration_cast<std::chrono::seconds>(
						  p1.time_since_epoch())
						  .count();
	std::string imgs_dir = "images_" + std::to_string(time);
	std::filesystem::create_directory(imgs_dir);

	while (true)
	{
		// const auto start_time = std::chrono::steady_clock::now();

		// Trigger Armed
		//    Continually check until trigger is armed. Once the trigger is armed, it
		//    is ready to be executed.
		// std::cout << TAB2 << "Wait until trigger is armed\n";
		bool triggerArmed = false;

		do
		{
			triggerArmed = Arena::GetNodeValue<bool>(pDevice->GetNodeMap(), "TriggerArmed");
		} while (triggerArmed == false);

		// Trigger an image
		//    Trigger an image manually, since trigger mode is enabled. This triggers
		//    the camera to acquire a single image. A buffer is then filled and moved
		//    to the output queue, where it will wait to be retrieved.
		// std::cout << TAB2 << "Trigger image\n";

		Arena::ExecuteNode(
			pDevice->GetNodeMap(),
			"TriggerSoftware");

		// Get image
		//    Once an image has been triggered, it can be retrieved. If no image has
		//    been triggered, trying to retrieve an image will hang for the duration
		//    of the timeout and then throw an exception.
		// std::cout << TAB2 << "Get image";

		Arena::IImage *pImage = pDevice->GetImage(TIMEOUT);

		const auto p1 = std::chrono::system_clock::now();
		const auto time = std::chrono::duration_cast<std::chrono::seconds>(
							  p1.time_since_epoch())
							  .count();
		const auto filename = imgs_dir + "/" + std::to_string(time) + "_image.png";
		std::cout << filename << std::endl;
		SaveImage(pImage, filename.c_str());

		// std::cout << " (" << pImage->GetWidth() << "x" << pImage->GetHeight() << ")\n";

		// requeue buffer
		// std::cout << TAB2 << "Requeue buffer\n";

		pDevice->RequeueBuffer(pImage);

		// const auto end_time = std::chrono::steady_clock::now();
		// const auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		// std::cout << "Took " << time_diff << " milliseconds to capture a photo" << std::endl;
	}

	// Stop the stream
	std::cout << TAB1 << "Stop stream\n";

	pDevice->StopStream();

	GenICam::gcstring sensorModeFinal = Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "SensorShutterMode");

	std::cout << sensorModeFinal << std::endl;

	// return nodes to their initial values
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "TriggerSource", triggerSourceInitial);
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "TriggerMode", triggerModeInitial);
	Arena::SetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), "TriggerSelector", triggerSelectorInitial);
}

// =-=-=-=-=-=-=-=-=-
// =- PREPARATION -=-
// =- & CLEAN UP =-=-
// =-=-=-=-=-=-=-=-=-

int main()
{
	// flag to track when an exception has been thrown
	bool exceptionThrown = false;

	std::cout << "Cpp_Trigger\n";

	try
	{
		// prepare example
		Arena::ISystem *pSystem = Arena::OpenSystem();
		pSystem->UpdateDevices(1000);
		std::vector<Arena::DeviceInfo> deviceInfos = pSystem->GetDevices();
		if (deviceInfos.size() == 0)
		{
			std::cout << "\nNo camera connected\nPress enter to complete\n";
			std::getchar();
			return 0;
		}
		Arena::IDevice *pDevice = pSystem->CreateDevice(deviceInfos[0]);

		// run example
		std::cout << "Commence example\n\n";
		ConfigureTriggerAndAcquireImage(pDevice);
		std::cout << "\nExample complete\n";

		// clean up example
		pSystem->DestroyDevice(pDevice);
		Arena::CloseSystem(pSystem);
	}
	catch (GenICam::GenericException &ge)
	{
		std::cout << "\nGenICam exception thrown: " << ge.what() << "\n";
		exceptionThrown = true;
	}
	catch (std::exception &ex)
	{
		std::cout << "Standard exception thrown: " << ex.what() << "\n";
		exceptionThrown = true;
	}
	catch (...)
	{
		std::cout << "Unexpected exception thrown\n";
		exceptionThrown = true;
	}

	std::cout << "Press enter to complete\n";
	std::getchar();

	if (exceptionThrown)
		return -1;
	else
		return 0;
}

void saveSettingsToFile(std::string filename, Arena::IDevice *pDevice)
{
	std::ofstream out;
	out.open(filename);

	out << "{" << std::endl;

	// traverse through all the nodes of the nodemap
	GenApi::NodeList_t nodes;
	pDevice->GetNodeMap()->GetNodes(nodes);
	for (GenApi::CCategoryPtr pCategoryNode : nodes)
	{
		if (!pCategoryNode)
		{
			continue;
		}
		GenApi::FeatureList_t children;
		pCategoryNode->GetFeatures(children);
		for (GenApi::CValuePtr pValue : children)
		{
			try
			{
				auto nodeName = pValue->GetNode()->GetName();
				// std::cout << nodeName << std::endl;
				switch (pValue->GetNode()->GetPrincipalInterfaceType())
				{
				case GenApi::intfIBoolean:
					out << "\t\"" << nodeName << "\": ";
					out << Arena::GetNodeValue<bool>(pDevice->GetNodeMap(), nodeName) << "," << std::endl;
					break;
				case GenApi::intfIString:
					out << "\t\"" << nodeName << "\": ";
					out << "\"" << Arena::GetNodeValue<GenICam::gcstring>(pDevice->GetNodeMap(), nodeName) << "\"," << std::endl;
					break;
				// case GenApi::intfIEnumeration:
				// 	out << "\t\"" << nodeName << "\": ";
				// 	out << "\"" << Arena::GetNodeValue<GenICam::>(pDevice->GetNodeMap(), nodeName) << "\"," << std::endl;
				// 	break;
				case GenApi::intfIInteger:
					out << "\t\"" << nodeName << "\": ";
					out << Arena::GetNodeValue<int64_t>(pDevice->GetNodeMap(), nodeName) << "," << std::endl;
					break;
				case GenApi::intfIFloat:
					out << "\t\"" << nodeName << "\": ";
					out << Arena::GetNodeValue<float64_t>(pDevice->GetNodeMap(), nodeName) << "," << std::endl;
					break;
					/*default:
						// std::cout << TAB3 << nodeName << " type not found\n";*/
				}
			}
			catch (GenICam::GenericException &ge)
			{
				std::cout << "\nGenICam exception thrown: " << ge.what() << "\n";
			}
			catch (std::exception &ex)
			{
				std::cout << "Standard exception thrown: " << ex.what() << "\n";
			}
			catch (...)
			{
				std::cout << "Unexpected exception thrown\n";
			}
		}
	}
	out << "}";
	out.close();
}