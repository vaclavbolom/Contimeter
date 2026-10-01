from ote import Ote
from datetime import date

ote = Ote()

prices = ote.getDayMarketPrices(date.today())

for p in prices:
    print(f'{p}: {prices[p]}')