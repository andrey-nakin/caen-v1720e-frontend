#ifndef	__CAEN_NONCOPYABLE_HXX__
#define	__CAEN_NONCOPYABLE_HXX__

namespace caen {

class Noncopyable {
public:

	Noncopyable() = default;
	~Noncopyable() = default;

private:

	Noncopyable(const Noncopyable&) = delete;
	Noncopyable& operator=(const Noncopyable&) = delete;

};

}

#endif	//	__CAEN_NONCOPYABLE_HXX__

