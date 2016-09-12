#ifndef BANDDATA_H
#define BANDDATA_H

#include <stdexcept>

#include <casacore/ms/MeasurementSets/MeasurementSet.h>

#include <casacore/tables/Tables/ArrayColumn.h>
#include <casacore/tables/Tables/ScalarColumn.h>

class BandData
{
	public:
		BandData() : _channelCount(0), _channelFrequencies(0), _frequencyStep(0.0)
		{
		}
		
		BandData(casacore::MSSpectralWindow &spwTable)
		{
			if(spwTable.nrow() != 1) throw std::runtime_error("Set should have exactly one spectral window");
			
			initFromTable(spwTable, 0);
		}
		
		BandData(casacore::MSSpectralWindow& spwTable, size_t bandIndex)
		{
			initFromTable(spwTable, bandIndex);
		}
		
		BandData(const BandData& source) : _channelCount(source._channelCount), _frequencyStep(source._frequencyStep)
		{
			_channelFrequencies = new double[_channelCount];
			for(size_t index = 0; index != _channelCount; ++index)
			{
				_channelFrequencies[index] = source._channelFrequencies[index];
			}
		}
		
		BandData(const BandData &source, size_t startChannel, size_t endChannel) :
			_channelCount(endChannel - startChannel), _frequencyStep(source._frequencyStep)
		{
			if(_channelCount == 0) throw std::runtime_error("No channels in set");
			
			_channelFrequencies = new double[_channelCount];
			for(size_t index = 0; index != _channelCount; ++index)
			{
				_channelFrequencies[index] = source._channelFrequencies[index + startChannel];
			}
		}
		
		~BandData()
		{
			delete[] _channelFrequencies;
		}
		
		void operator=(const BandData& source)
		{
			_channelCount = source._channelCount;
			_frequencyStep = source._frequencyStep;
			if(_channelCount != 0)
			{
				_channelFrequencies = new double[_channelCount];
				for(size_t index = 0; index != _channelCount; ++index)
				{
					_channelFrequencies[index] = source._channelFrequencies[index];
				}
			}
			else {
				_channelFrequencies = 0;
			}
		}
		
		size_t ChannelCount() const { return _channelCount; }
		
		double ChannelFrequency(size_t channelIndex) const
		{
			return _channelFrequencies[channelIndex];
		}
		double ChannelWavelength(size_t channelIndex) const
		{
			return 299792458.0L / _channelFrequencies[channelIndex];
		}
		double HighestFrequency() const
		{
			return _channelFrequencies[_channelCount-1];
		}
		double LowestFrequency() const
		{
			return _channelFrequencies[0];
		}
		double CentreFrequency() const
		{
			return (HighestFrequency() + LowestFrequency()) * 0.5;
		}
		static double FrequencyToLambda(double frequencyHz)
		{
			return 299792458.0L / frequencyHz;
		}
		double CentreWavelength() const
		{
			return 299792458.0L / ((HighestFrequency() + LowestFrequency()) * 0.5);
		}
		double FrequencyStep() const
		{
			return _frequencyStep;
		}
		double LongestWavelength() const
		{
			return ChannelWavelength(0);
		}
		double SmallestWavelength() const
		{
			return ChannelWavelength(_channelCount-1);
		}
		double BandStart() const
		{
			return LowestFrequency() - FrequencyStep()*0.5;
		}
		double BandEnd() const
		{
			return HighestFrequency() + FrequencyStep()*0.5;
		}
		double Bandwidth() const
		{
			return HighestFrequency() - LowestFrequency() + FrequencyStep();
		}
		
	private:
		void initFromTable(casacore::MSSpectralWindow& spwTable, size_t bandIndex)
		{
			casacore::ROScalarColumn<int> numChanCol(spwTable, casacore::MSSpectralWindow::columnName(casacore::MSSpectralWindowEnums::NUM_CHAN));
			int temp;
			numChanCol.get(bandIndex, temp);
			_channelCount = temp;
			if(_channelCount == 0) throw std::runtime_error("No channels in set");
			
			casacore::ROArrayColumn<double> chanFreqCol(spwTable, casacore::MSSpectralWindow::columnName(casacore::MSSpectralWindowEnums::CHAN_FREQ));
			casacore::Array<double> channelFrequencies;
			chanFreqCol.get(bandIndex, channelFrequencies, true);
			
			_channelFrequencies = new double[_channelCount];
			size_t index = 0;
			for(casacore::Array<double>::const_iterator i=channelFrequencies.begin();
					i != channelFrequencies.end(); ++i)
			{
				_channelFrequencies[index] = *i;
				++index;
			}
		  if(_channelCount > 1)
		    _frequencyStep = _channelFrequencies[1] - _channelFrequencies[0];
		  else
		    _frequencyStep = 0.0;
		}
		
		size_t _channelCount;
		double *_channelFrequencies;
		double _frequencyStep;
};

#endif
