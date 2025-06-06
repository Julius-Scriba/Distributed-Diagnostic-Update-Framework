# ULTSPY Web UI

This web interface allows operators to interact with the ULTSPY C2 backend. It is built with Vite, React, TypeScript and TailwindCSS. The UI components follow the shadcn/ui style guidelines.

## Setup

```bash
npm install
npm run dev
```

The development server proxies API requests to `http://localhost:5000`.

## Login

Open `/login` and enter your API key. It will be stored in `localStorage` under
`ULTSPY_API_KEY`. All requests to `/admin/*` automatically include this key in
the `X-API-KEY` header via an Axios interceptor.
If a request fails with `401 Unauthorized`, the key is removed and you are
redirected back to the login page. Network outages trigger a red banner saying
"Verbindung zum Backend unterbrochen." at the top of the dashboard.
Use the **Logout** button in the header to clear the stored key manually.

## Project Structure

- `src/layout` contains the shared `Header`, `Sidebar` and `Layout` components.
- `src/pages` defines the routes:
  - **Dashboard** – landing page
  - **Agents** – lists `/admin/agents` with live updates every 15 seconds
  - **Commands** – manage the queued commands per agent
  - **Logs** – display recon reports and server logs per agent
  - **Settings** – displays the current backend configuration via `/admin/config`
  - **Templates** – manage reusable command templates

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
using `date-fns` for relative timestamps.

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

Invalid JSON parameters are reported to the user. All requests include the stored API key via the `X-API-KEY` header.

## Logs Interface

On the **Logs** page operators can inspect recon reports and other server logs for a selected agent.

```
GET /admin/logs/<uuid>
-> { "logs": [{"timestamp":"...","type":"Recon","description":"Recon data","data":"{...}"}] }
```

Select an agent from the dropdown to load its logs. Each row shows a timestamp, the log type and a short description. If raw data is available it can be expanded in place.

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

## Production Build

Create an optimized build before deploying:

```bash
npm install
npm run build
```

The compiled files appear under `dist/` and can be served by Nginx or another static server.
