from zeep import Client
from zeep.helpers import serialize_object

wsdl = "https://www.ote-cr.cz/services/PublicDataService?wsdl"

client = Client(wsdl=wsdl)

# print available methods
print(client.service)

response = client.service.GetImPriceE(
    StartDate="2026-03-21",
    EndDate="2026-03-21",
    StartHour=1,
    EndHour=24
)

print(serialize_object(response))