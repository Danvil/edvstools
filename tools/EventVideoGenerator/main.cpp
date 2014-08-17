#include <Edvs/EventIO.hpp>
#include "lodepng.h"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

struct mat8
{
	std::vector<unsigned char> data;
	unsigned int rows, cols;
	
	mat8(unsigned int nrows, unsigned int ncols)
	: rows(nrows), cols(ncols), data(nrows*ncols) {}

	unsigned char operator()(int i, int j) const {
		return data[cols*i + j];
	}

	unsigned char& operator()(int i, int j) {
		return data[cols*i + j];
	}
};

void save_png(const std::string& filename, const mat8& img)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, img.data, img.cols, img.rows, LCT_GREY, 8);
	//if there's an error, display it
	if(error) {
		std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
	}
}

const unsigned int RETINA_SIZE = 128;

unsigned int clip_retina_coord(float u)
{
	return static_cast<unsigned int>(
		std::min<int>(RETINA_SIZE-1,
			std::max<int>(0,
				static_cast<int>(std::floor(0.5f + u)))));
}

void create_video(const std::vector<Edvs::Event>& events, uint64_t dt, uint64_t decay, boost::format fmt_fn, bool skip_empty, uint8_t id=0)
{
	mat8 retina(RETINA_SIZE, RETINA_SIZE);
	uint64_t frametime = events.front().t + dt;
	auto it_begin = events.begin();
	unsigned frame_save_id = 0;
	for(unsigned frame=0; it_begin!=events.end(); frame++, frametime+=dt) {
		// prepare frame
		std::fill(retina.data.begin(), retina.data.end(), 128);
		// find first and last event
		uint64_t t_begin = (frametime <= decay) ? 0 : frametime - decay;
		auto time_cmp = [](const Edvs::Event& e, uint64_t t) { return e.t < t; };
		it_begin = std::lower_bound(it_begin, events.end(), t_begin, time_cmp);
		auto it_end = std::lower_bound(it_begin, events.end(), frametime, time_cmp);
		// skip empty frames
		if(skip_empty && it_begin == it_end) {
			continue;
		}
		// paint events
		for(auto it=it_begin; it!=it_end; ++it) {
			const Edvs::Event& event = *it;
			if(event.id != id) {
				continue;
			}
			unsigned int x = clip_retina_coord(event.x);
			unsigned int y = clip_retina_coord(event.y);
			unsigned char d = static_cast<unsigned>(127.0f*static_cast<float>(frametime - event.t)/static_cast<float>(decay));
			unsigned char c = (event.parity ? 255-d : d);
			retina(y, x) = c;
		}
		std::cout << "Frame " << frame << ": time=" << frametime << ", #events=" << std::distance(it_begin, it_end) << std::endl;
		save_png((fmt_fn % frame_save_id).str(), retina);
		frame_save_id ++;
	}
}


struct MatRGB
{
	std::vector<unsigned char> data;
	unsigned int rows, cols;
	
	MatRGB(unsigned int nrows, unsigned int ncols)
	: rows(nrows), cols(ncols), data(3*nrows*ncols) {}

	unsigned char operator()(int i, int j, int k) const {
		return data[3*(cols*i + j) + k];
	}

	unsigned char& operator()(int i, int j, int k) {
		return data[3*(cols*i + j) + k];
	}
};

void save_png(const std::string& filename, const MatRGB& img)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, img.data, img.cols, img.rows, LCT_RGB, 8);
	//if there's an error, display it
	if(error) {
		std::cout << "encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
	}
}

struct ColoredEvent
{
	uint64_t t;
	float x, y;
	unsigned polarity;
	unsigned id;
	float depth;
	float disparity;
};

std::vector<ColoredEvent> LoadColoredEvents(const std::string& fn)
{
	std::vector<ColoredEvent> u;
	u.reserve(1000000);
	std::ifstream ifs(fn);
	while(true) {
		ColoredEvent e;
		ifs >> e.t >> e.x >> e.y >> e.polarity >> e.id >> e.depth >> e.disparity;
		if(!ifs.eof()) {
			u.push_back(e);
		}
		else {
			break;
		}
	}
	return u;
}

inline
void Color(const Eigen::Vector3f& col, unsigned char& cr, unsigned char& cg, unsigned char& cb)
{
	cr = static_cast<unsigned char>(255.0f*col[0]);
	cg = static_cast<unsigned char>(255.0f*col[1]);
	cb = static_cast<unsigned char>(255.0f*col[2]);
}
template<bool WRAP, unsigned N>
Eigen::Vector3f ColorScheme(const Eigen::Vector3f colors[N], float a) {
	static_assert(N >= 2, "Number of colors must be at least 2");
	constexpr unsigned Q = WRAP ? N : N-1;
	const float v = static_cast<float>(Q) * std::min(std::max(0.0f, a), 1.0f);
	const unsigned i1 = std::min(static_cast<unsigned>(v), Q-1);
	const unsigned i2 = WRAP ? (i1 + 1) % N : i1 + 1;
	const float p = v - static_cast<float>(i1);
	return (1.0f-p)*colors[i1] + p*colors[i2];
}
inline
Eigen::Vector3f ColorSchemeDarkRainbow(float p)
{
	constexpr unsigned N = 11;
	static const Eigen::Vector3f colors[N] = {
		{0.237736, 0.340215, 0.575113},
		{0.253651, 0.344893, 0.558151},
		{0.264425, 0.423024, 0.3849},
		{0.291469, 0.47717, 0.271411},
		{0.416394, 0.555345, 0.24182},
		{0.624866, 0.673302, 0.264296},
		{0.813033, 0.766292, 0.303458},
		{0.877875, 0.731045, 0.326896},
		{0.812807, 0.518694, 0.303459},
		{0.72987, 0.239399, 0.230961},
		{0.72987, 0.239399, 0.230961}};
	return ColorScheme<false,N>(colors, p);
}
inline
Eigen::Vector3f ColorizeDisparity(float d)
{
	if(d == -1) {
		return {0,0,0};
	}
	if(d < -1) {
		return {1,0,1};
	}
	constexpr unsigned DISP_MAX = 32; // FIXME
	float p = d / static_cast<float>(DISP_MAX-1);
	return ColorSchemeDarkRainbow(p);
}
inline
Eigen::Vector3f Decay(const Eigen::Vector3f& a, float p)
{
	const Eigen::Vector3f target{1,1,1};
	return (1.0f-p)*a + p*target;
}
inline
uint64_t DeltaT(uint64_t a, uint64_t b)
{ return std::max<int64_t>(0, static_cast<int64_t>(b) - static_cast<int64_t>(a)); }

void create_video(const std::vector<ColoredEvent>& events, uint64_t dt, uint64_t decay, boost::format fmt_fn, bool skip_empty)
{
	MatRGB retina(RETINA_SIZE, RETINA_SIZE);
	uint64_t frametime = events.front().t + dt;
	auto it_begin = events.begin();
	unsigned frame_save_id = 0;
	for(unsigned frame=0; it_begin!=events.end(); frame++, frametime+=dt) {
		// prepare frame
		std::fill(retina.data.begin(), retina.data.end(), 255);
		// find first and last event
		uint64_t t_begin = (frametime <= decay) ? 0 : frametime - decay;
		auto time_cmp = [](const ColoredEvent& e, uint64_t t) { return e.t < t; };
		it_begin = std::lower_bound(it_begin, events.end(), t_begin, time_cmp);
		auto it_end = std::lower_bound(it_begin, events.end(), frametime, time_cmp);
		// skip empty frames
		if(skip_empty && it_begin == it_end) {
			continue;
		}
		// paint events
		for(auto it=it_begin; it!=it_end; ++it) {
			const auto& event = *it;
			if(event.id != 0) continue;
			unsigned int x = clip_retina_coord(event.x);
			unsigned int y = clip_retina_coord(event.y);
			float p = std::min(1.0f,static_cast<float>(frametime - event.t)/static_cast<float>(decay));
			unsigned char cr, cg, cb;
			Color(Decay(ColorizeDisparity(event.disparity),p), cr, cg, cb);
			retina(y, x, 0) = cr;
			retina(y, x, 1) = cg;
			retina(y, x, 2) = cb;
		}
		std::cout << "Frame " << frame << ": time=" << frametime << ", #events=" << std::distance(it_begin, it_end) << std::endl;
		save_png((fmt_fn % frame_save_id).str(), retina);
		frame_save_id ++;
	}
}

int main(int argc, char** argv)
{
	std::string p_fn = "/home/david/Documents/DataSets/edvs_raoul_mocap_2/33/events";
	std::string p_dir = "/media/tmp/edvs_video";
	uint64_t p_dt = 1000000/25;
	uint64_t p_decay = 100*1000;
	bool p_skip_empty = false;
	unsigned p_id = 0;
	bool p_colored = false;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("fn", po::value(&p_fn), "filename for input event file")
		("dir", po::value(&p_dir), "filename for output directory")
		("dt", po::value(&p_dt)->default_value(p_dt), "frame time increase in microseconds")
		("decay", po::value(&p_decay)->default_value(p_decay), "displayed time per frame in microseconds")
		("colored", po::value(&p_colored)->default_value(p_colored), "set to true to parse events with color")
		("noempty", po::value(&p_skip_empty), "whether to skip empty frames")
		("id", po::value(&p_id)->default_value(p_id), "sensor id")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help") || !vm.count("fn") || !vm.count("dir")) {
		std::cout << desc << std::endl;
		return 1;
	}

	if(vm.count("colored")) {
		// read events
		auto events = LoadColoredEvents(p_fn);
		std::cout << "Read " << events.size() << " events" << std::endl;
		// create video
		boost::format fmt_fn(p_dir + "/%05d.png");
		create_video(events, p_dt, p_decay, fmt_fn, p_skip_empty); // TODO p_id
	}
	else {
		// read events
		auto events = Edvs::LoadEvents(p_fn);
		std::cout << "Read " << events.size() << " events" << std::endl;
		// create video
		boost::format fmt_fn(p_dir + "/%05d.png");
		create_video(events, p_dt, p_decay, fmt_fn, p_skip_empty, p_id);
	}

	// hit for ffmpeg
	std::cout << "Run the following command to create the video:" << std::endl;
	std::cout << "cd " << p_dir << std::endl;
	std::cout << "mogrify -format jpg *.png" << std::endl;
	float fps = 1000000.0f/static_cast<float>(p_dt);
	std::cout << "avconv -f image2 -r " << fps << " -i %05d.jpg -c:v libx264 -r " << fps << " video.mp4" << std::endl;

	return 1;
}
