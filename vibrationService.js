const moment = require('moment');
const _ = require('lodash');

const threshold = 5;
const measuringDuration = moment.duration(30, 'seconds');
const minimumPoints = 10;
const maximumHistory = 10000;

let values = [];

const VibrationService = {
  addPosition(value) {
    const date = moment();
    console.log("Adding position", value, date)

    values = _.takeRight(
      _.concat(values, [{date, value}]),
      maximumHistory
    );
  },
  getPositionsAfter(afterDate) {
    return _.filter(values, ({ date, value }) => date.isSameOrAfter(afterDate));
  },
  isVibrating() {
    const measuredPositions = this.getPositionsAfter(
      moment().subtract(measuringDuration)
    );

    if (measuredPositions.length < minimumPoints) {
      console.log("NOT enough points for checking if it vibrates");
      return false;
    }

    const stdDev = this.getStandardDeviation(
      _.map(measuredPositions, ({value}) => value)
    );
    const isVibrating = stdDev > threshold;

    console.log("Standard Deviation is", stdDev);
    console.log("Is vibrating:", isVibrating);

    return isVibrating;
  },
  getLastPosition() {
    return _.last(values);
  },
  getStandardDeviation(values) {
    const sum = _.sum(values);
    const average = sum / values.length;

    const averageSquareDeviation = _.reduce(values, (acc, value) => {
      return acc + (value - average) * (value - average)
    }, 0) / values.length;

    return Math.sqrt(averageSquareDeviation);
  }
}

module.exports = VibrationService;