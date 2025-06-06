# ULTSPY Web UI

This web interface allows operators to interact with the ULTSPY C2 backend. It is built with Vite, React, TypeScript and TailwindCSS. The UI components follow the shadcn/ui style guidelines.

## Setup

```bash
npm install
npm run dev
```

The development server proxies API requests to `http://localhost:5000`.

## Login

Open `/login` and enter your API key. The key is verified by the backend and a
JWT token is returned. This token is saved in `localStorage` under
`ULTSPY_JWT` and automatically attached to all `/admin/*` requests via the
`Authorization` header. If a request fails with `401 Unauthorized`, the token is
removed and you are redirected back to the login page. Network outages trigger a
red banner saying "Verbindung zum Backend unterbrochen." at the top of the
dashboard. Use the **Logout** button in the header to clear the token manually.

## Project Structure

- `src/layout` contains the shared `Header`, `Sidebar` and `Layout` components.
- `src/pages` defines the routes:
  - **Dashboard** – landing page
  - **Agents** – lists `/admin/agents` with live updates every 15 seconds
  - **Commands** – manage the queued commands per agent
  - **Surveillance** – system-wide list of surveillance images
  - **Logs** – display recon reports and server logs per agent
  - **Settings** – displays the current backend configuration via `/admin/config`
  - **Templates** – manage reusable command templates
  - **API Keys** – create and revoke login keys
  - **Audit Log** – view administrative login and key events

Navigation is handled via React Router DOM and styled with TailwindCSS.

## User Experience

The sidebar and header remain visible using sticky positioning so navigation is
always available. Loading indicators appear centered on each page and every
route distinguishes loading, success and error states. Tables become scrollable
on small screens and the active navigation link is highlighted with a subtle
hover effect.

## Live Agent Heartbeat

The **Agents** page polls `/admin/agents` every 15 seconds to keep the list
current. The "Last Seen" column shows how long ago an agent checked in
using `date-fns` for relative timestamps. A **Stability** indicator visualizes
how reliable the heartbeat is:

| Delta | Indicator |
| ----- | --------- |
| ≤30s  | 🟢 Stable |
| 31–90s| 🟡 Warning |
| 91–180s| 🟠 Degraded |
| >180s or offline | 🔴 Offline |

Click on an agent UUID to open its detail page `/agents/:uuid` showing the
stability badge, last seen information and a dialog to send commands directly.
Below the dialog a list of **Command Templates** is available. Selecting one
previews the JSON payload and allows sending it to the agent with a single
click.  A scrollable **Logs** table underneath lists the entire log history of
the agent. Each row shows a relative timestamp, type and description. Large JSON
payloads can be expanded using a "mehr anzeigen" toggle. A second **Surveillance**
tab filters these logs for entries of type `Surveillance` and displays detected
person names with image thumbnails. Clicking a thumbnail opens a lightbox with
the full picture and the raw JSON payload can also be expanded.

## Commands Interface

Open the **Commands** page to inspect or modify the command queue for a specific agent.

1. Choose an agent from the dropdown. The queue is loaded from:
   ```
   GET /admin/commands/<uuid>
   -> { "commands": [{"command":"RECON","parameters":{},"timestamp":"..."}] }
   ```
2. Add a new command by selecting a command name and providing parameters as JSON. The request is sent to:
   ```
   POST /admin/command/<uuid>
   { "command": "RECON", "parameters": {}}
   ```
   A confirmation is shown on success.

Invalid JSON parameters are reported to the user. All requests include the stored JWT via the `Authorization` header.

## Logs Interface

On the **Logs** page operators can inspect recon reports and other server logs for a selected agent.

```
GET /admin/logs/<uuid>
-> { "logs": [{"timestamp":"...","type":"Recon","description":"Recon data","data":"{...}"}] }
```

Select an agent from the dropdown to load its logs. Each row shows a timestamp, the log type and a short description. If raw data is available it can be expanded in place.
On the agent detail page you can switch to a **Surveillance** tab that filters the list for surveillance entries and shows thumbnails of captured images.

## Surveillance Overview

Open the **Surveillance** page from the sidebar to review all surveillance images across every agent.

```typescript
GET /admin/agents
GET /admin/logs/<uuid>
-> filter logs for type "Surveillance"
```

Each row lists the agent UUID, a relative timestamp and the detected person (if present). Thumbnails open a lightbox with the full picture.

## Settings Interface

The **Settings** page fetches the current backend configuration using `/admin/config`.
Heartbeat timeouts, allowed hosts and other details are displayed in a table and the build versions are shown at the top.

## Templates Interface

The **Templates** page manages reusable command definitions that can be quickly
queued for agents.

```
GET /admin/templates
-> { "templates": [{"template_id":"...","name":"Recon","command":"RECON","parameters":{}}] }

POST /admin/templates
{ "name": "Recon", "command": "RECON", "parameters": {} }

DELETE /admin/templates/<template_id>
```

Templates consist of a name, a command and optional JSON parameters. Duplicate
names are rejected with `409 Conflict`.

## API Keys Interface

The **API Keys** page lists all active keys and allows creation of new ones.
Only once after creation the secret value is displayed. Existing keys can be
revoked which immediately prevents further logins. Each entry shows when the key was last used and from which IP the last login erfolgte.

The **Audit Log** page under Settings displays a table of recent administrative actions such as logins and key changes.

## Production Build

Create an optimized build before deploying:

```bash
npm install
npm run build
```

The compiled files appear under `dist/` and can be served by Nginx or another static server.
