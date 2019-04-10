#pragma once

#include <iostream>
#include <memory>
#include <Converter.hxx>

namespace gdc {

namespace converter {

class FilePerRun: public Converter {
protected:

	FilePerRun();

public:

	void BeginRun(int transition, int run, int time) override;
	void EndRun(int transition, int run, int time) override;
	bool ProcessMidasEvent(TDataContainer& dataContainer) override;

	virtual bool ProcessMidasEvent(std::ostream& dest,
			TDataContainer& dataContainer) = 0;

	virtual void ConstructFileName(std::ostream& dest, int run) const;
	virtual std::string FileExtension() const = 0;
	virtual std::ios_base::openmode FileMode() const;

	int GetRun() const;

private:

	int run;
	std::unique_ptr<std::ostream> dest;

};

}

}
