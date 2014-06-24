#include <Edvs/EventIO.hpp>
#include "lodepng.h"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <iostream>
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

void create_video(const std::vector<Edvs::Event>& events, uint64_t dt, boost::format fmt_fn, bool skip_empty, uint8_t id=0)
{
	mat8 retina(RETINA_SIZE, RETINA_SIZE);
	uint64_t tbase = events.front().t;
	auto it = events.begin();
	unsigned frame_save_id = 0;
	for(unsigned int frame=0; it!=events.end(); frame++) {
		std::fill(retina.data.begin(), retina.data.end(), 128);
		unsigned int num = 0;
		while(it != events.end()) {
			const Edvs::Event& event = *it;
			if(event.id != id) {
				it++;
				continue;
			}
			uint64_t t = event.t;
			if(t >= tbase + dt) {
				break;
			}
			unsigned char p = static_cast<unsigned char>(127.0f*static_cast<float>(t - tbase)/static_cast<float>(dt));
			unsigned int x = clip_retina_coord(event.x);
			unsigned int y = clip_retina_coord(event.y);
			unsigned char c = (event.parity ? 127+p : 127-p);
			retina(y, x) = c;
			it++;
			num++;
		}
		std::cout << "Frame " << frame << ": time=" << it->t << ", #events=" << num << std::endl;
		if(num > 0) {
			save_png((fmt_fn % frame_save_id).str(), retina);
			frame_save_id ++;
		}
		tbase += dt;
	}
}

int main(int argc, char** argv)
{
	std::string p_fn = "/home/david/Documents/DataSets/edvs_raoul_mocap_2/33/events";
	std::string p_dir = "/media/tmp/edvs_video";
	uint64_t p_skip = 0;
	int64_t p_dt = 1000000/25;
	bool p_skip_empty = false;
	unsigned p_id = 0;

	namespace po = boost::program_options;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("fn", po::value(&p_fn), "filename for input event file")
		("dir", po::value(&p_dir), "filename for output directory")
		("dt", po::value(&p_dt)->default_value(p_dt), "time per frame in microseconds")
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

	// read events
	std::vector<Edvs::Event> events = Edvs::LoadEvents(p_fn);
	std::cout << "Read " << events.size() << " events" << std::endl;

	// skip events
	// FIXME
	if(p_skip != 0) {
		std::cerr << "not implemented" << std::endl;
		std::exit(0);
	}

	// create video
	boost::format fmt_fn(p_dir + "/%05d.png");
	create_video(events, p_dt, fmt_fn, p_skip_empty, p_id);

	// hit for ffmpeg
	std::cout << "Run the following command to create the video:" << std::endl;
	std::cout << "cd " << p_dir << std::endl;
	std::cout << "mogrify -format jpg *.png" << std::endl;
	float fps = 1000000.0f/static_cast<float>(p_dt);
	std::cout << "avconv -f image2 -r " << fps << " -i %05d.jpg -c:v libx264 -r " << fps << " video.mp4" << std::endl;

	return 1;
}
