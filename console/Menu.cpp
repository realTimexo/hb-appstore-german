#include "Menu.hpp"
#include "../gui/main.hpp"
#include <algorithm>
#include <sstream>

using namespace std;

Menu::Menu(Console* console)
{
	this->console = console;
	this->position = 0;
	this->screen = SPLASH;
}

void Menu::display()
{
	if (this->screen == SPLASH)
	{
		console->drawString(9, 21, "Homebrew App Store (Wiederherstellungsmodus)");
		console->drawColorString(9, 26, "Danke an:", 0xcc, 0xcc, 0xcc);
		console->drawColorString(15, 27, "vgmoose, pwsincd, rw-r-r_0644, zarklord", 0xcc, 0xcc, 0xcc);
		console->drawColorString(9, 32, "Drücke [A] um Pakete zu verwalten", 0xff, 0xff, 0x00);
    console->drawColorString(9, 34, "Drücke [Y] um Konfiguration zurückzusetzen", 0xff, 0xff, 0x00);
	}

  if (this->screen == RECOVERY_OPTIONS)
  {
		console->fillRect(0, 0, 80, 1, 0, 0, 255);
		console->drawColorString(80 / 2 - 12, 0, "Konfiguration zurücksetzen", 0xff, 0xff, 0xff);

		console->drawString(9, 16, "Wenn jede POMMES perfekt wäre,");
		console->drawString(9, 18, "bräuchten wir keine CHIPS!");

		console->drawColorString(9, 23, "Halte [L]+[R]+[A] um Konfiguration zurückzusetzen", 0xff, 0xff, 0x00);

		console->drawString(9, 28, "Problem melden unter: github.com/fortheusers/hb-appstore");

		console->fillRect(0, 44, 80, 1, 0, 0, 255);
		console->drawColorString(0, 44, "[B] Zurück", 0xff, 0xff, 0xff);
  }

	if (this->get == NULL && this->screen != SPLASH && this->screen != RECOVERY_OPTIONS
      && this->screen != INSTALL_SUCCESS && this->screen != INSTALL_FAILED)
	{
		// if libget isn't initialized, and we're trying to load a get-related screen, init it!
		console->update();
		console->drawString(3, 11, "Synchronisiere Paketinformationen...");
		console->drawColorString(3, 13, "Einen Moment Geduld!", 0, 0xcc, 0xcc);
		console->update();
		initGet();
		return;
	}

	if (this->screen == LIST_MENU || this->screen == INSTALL_SCREEN)
	{
		// draw the top bar
		console->fillRect(0, 0, 80, 1, 255, 255, 0);
		console->drawColorString(80 / 2 - 15, 0, "Homebrew App Store Wiederherstellung", 0, 0, 0);

		// draw bottom bar
		console->fillRect(0, 44, 80, 1, 255, 255, 0);
		console->drawColorString(0, 44, "[A] Installieren   [B] Zurück", 0, 0, 0);
		console->drawColorString(80 - (strlen(this->repoUrl) + 2), 44, this->repoUrl, 0, 0, 0);
	}

	if (this->screen == LIST_MENU)
	{
		int start = (this->position / PAGE_SIZE) * PAGE_SIZE; //

		// go through this page of apps until the end of the page, or longer than the packages list
		for (int x = start; x < start + PAGE_SIZE && x < get->getPackages().size(); x++)
		{
			int curPosition = (x % PAGE_SIZE) * 3 + 2;

			auto cur = get->list()[x];
			std::stringstream line;
			line << cur.getTitle() << " (" << cur.getVersion() << ")";
			console->drawString(15, curPosition, line.str().c_str());

			auto status = cur.getStatus();

			int r = (status == UPDATE || status == LOCAL) ? 0xFF : 0x00;
			int g = (status == UPDATE) ? 0xF7 : 0xFF;
			int b = (status == INSTALLED || status == LOCAL) ? 0xFF : 0x00;
			console->drawColorString(5, curPosition, cur.statusString(), r, g, b);

			std::stringstream line2;
			line2 << cur.getShortDescription() << " [" << cur.getAuthor() << "]";

			console->drawColorString(16, curPosition + 1, line2.str().c_str(), 0xcc, 0xcc, 0xcc);
		}

		std::stringstream footer;
		footer << "Seite " << this->position / PAGE_SIZE + 1 << " von " << (get->getPackages().size()-1) / PAGE_SIZE + 1;
		console->drawString(34, 40, footer.str().c_str());
		console->drawColorString(15, 42, "Benutze links/rechts und oben/unten zum Navigieren", 0xcc, 0xcc, 0xcc);

		console->drawString(1, (this->position % PAGE_SIZE) * 3 + 2, "-->");
	}

	if (this->screen == INSTALL_SCREEN)
	{
		if (this->position < 0 || this->position > get->getPackages().size())
		{
			// invalid selection, go back a screen
			this->screen--;
			return;
		}

		// currently selected package
		auto cur = get->list()[this->position];

		console->drawString(5, 3, cur.getTitle().c_str());
		console->drawString(6, 5, cur.getVersion().c_str());
		console->drawString(6, 6, cur.getAuthor().c_str());

		auto status = cur.getStatus();

		int r = (status == UPDATE || status == LOCAL) ? 0xFF : 0x00;
		int g = (status == UPDATE) ? 0xF7 : 0xFF;
		int b = (status == INSTALLED || status == LOCAL) ? 0xFF : 0x00;
		console->drawColorString(5, 8, cur.statusString(), r, g, b);

		console->drawColorString(5, 12, "Drücke [A] um dieses Paket zu installieren", 0xff, 0xff, 0x00);

		if (status != GET && status != LOCAL)
			console->drawColorString(5, 14, "Drücke [X] um dieses Paket zu entfernen", 0xff, 0xff, 0x00);

		console->drawString(5, 16, "Drücke [B] um zurückzugehen");
	}

	if (this->screen == INSTALLING || this->screen == REMOVING)
	{
		// currently selected package
		auto cur = get->list()[this->position];

		console->drawString(5, 4, cur.getTitle().c_str());

		if (this->screen == INSTALLING)
			console->drawColorString(5, 5, "Lade Paket herunter...", 0xff, 0xff, 0x00);
		else
			console->drawColorString(5, 5, "Entferne Paket...", 0xff, 0xff, 0x00);

    console->drawString(5, 9, "Kein Fortschrittsbalken in diesem Modus verfügbar");
		console->drawColorString(5, 11, "Bitte warten!", 0, 0xcc, 0xcc);

		console->drawString(5, 15, "Problem melden unter: github.com/fortheusers/hb-appstore");

	}

	if (this->screen == INSTALL_SUCCESS || this->screen == INSTALL_FAILED)
	{
		if (this->screen == INSTALL_SUCCESS)
			console->drawColorString(3, 12, "Erfolgreich ausgeführt!", 0, 0xff, 0);
		else
		{
			console->drawColorString(3, 10, "Fehlgeschlagen", 0xff, 0, 0);
			console->drawString(3, 12, "Du kannst ein Problem melden unter github.com/fortheusers/hb-appstore");
		}

		console->drawColorString(3, 14, "Drücke [A] um fortzufahren", 0xff, 0xff, 0x00);
	}

	console->update();
}

void Menu::initGet()
{
	// this is a blocking load
#if defined(WII)
	// default the repo type to OSC for wii
	this->get = new Get(DEFAULT_GET_HOME, DEFAULT_REPO, true, "osc");
#else
	this->get = new Get(DEFAULT_GET_HOME, DEFAULT_REPO);
#endif

	// recovery GUI does not check the metarepo for updates

	if (get->getRepos().size() > 0)
		this->repoUrl = get->getRepos()[0]->getUrl().c_str();
	else
		this->repoUrl = "Keine Pakete in den Repositories gefunden!";
}

void Menu::moveCursor(int diff)
{
	if (this->get == NULL) return;

	int old_position = position;
	this->position += diff;

	if (position < 0)
	{
		// went back too far, wrap around to last package
		position = get->getPackages().size() - 1;
	}

	else if (position >= get->getPackages().size())
	{
		// too far forward, wrap around to first package
		position = 0;
	}
}

void Menu::advanceScreen(bool advance)
{
	if (advance)
	{
		// A on these screens just returns to app list
		if (this->screen == INSTALL_SUCCESS || this->screen == INSTALL_FAILED)
		{
			this->screen = LIST_MENU;
		}
		// if on the install screen, or install-related screens, A does nothing
		else if (this->screen < INSTALLING)
		{
			// just advance the screen
			this->screen++;
		}
	}
	else
	{
		// go back, unless we can't anymore, or doing an install-related action
		if (this->screen > SPLASH && this->screen < INSTALLING)
			this->screen--;
	}
	printf("current screen: %d\n", this->screen);
}
