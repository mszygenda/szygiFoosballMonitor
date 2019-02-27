const moment = require('moment');
const _ = require('lodash');

const MEASUREMENT_DURATION = moment.duration(60, 'seconds');
const DEVIATION_THRESHOLD = 5;
const MINIMUM_POINTS = 20;
const MAXIMUM_HISTORY_SIZE = 10000;

let values = [];

const VibrationService = {
  addPosition(value) {
    const date = moment();
    console.log("Adding position", value, date)

    values = _.takeRight(
      _.concat(values, [{date, value}]),
      MAXIMUM_HISTORY_SIZE
    );
  },
  getPositionsAfter(afterDate) {
    return _.filter(values, ({ date, value }) => date.isSameOrAfter(afterDate));
  },
  isVibrating() {
    const measuredPositions = this.getPositionsAfter(
      moment().subtract(MEASUREMENT_DURATION)
    );

    if (measuredPositions.length < MINIMUM_POINTS) {
      console.log("NOT enough points for checking if it vibrates");
      return false;
    }

    const stdDev = this.getStandardDeviation(
      _.map(measuredPositions, ({value}) => value)
    );
    const isVibrating = stdDev > DEVIATION_THRESHOLD;

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