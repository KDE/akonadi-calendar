# Akonadi Calendar #

Akonadi Calendar is a library that effectively bridges the type-agnostic API of
the Akonadi client libraries and the domain-specific KCalCore library. It provides
jobs, models and other helpers to make working with events and calendars through
Akonadi easier.

The most notable class is Akonadi::ETMCalendar, a model built on top of the
Akonadi::EntityTreeModel which provides filters to only show events from selected
calendars, iterate over events, including recurrences, provides reverse lookup
from KCalCore::Incidence to Akonadi::Item and other features.

Besides the model there is for example Akonadi::FreeBusyProviderBase, an interface
for Akonadi Resources that can provide Free/Busy information.
