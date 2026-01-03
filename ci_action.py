#!/usr/bin/env python3
"""
Entry point for all the CI actions.
"""

import logging
from argparse import ArgumentParser
from pathlib import Path

from ci import actions
from ci.logging_utils import setup_logging

root = Path(__file__).resolve().parent
log = logging.getLogger(__name__)

actions = {
    "determine_docker_image": actions.define_docker_image,
    "build": actions.build_project,
    "run_tests": actions.run_tests,
    "deploy": actions.deploy,
    "clean": actions.clean,
}


def main():
    setup_logging()
    parser = ArgumentParser("CI Action Selector")
    parser.add_argument(
        "action", type=str, choices=actions.keys(), help="The CI action to perform."
    )
    parser.add_argument("preset", type=str, help="The preset to check.")
    args = parser.parse_args()

    if args.action not in actions:
        log.error(f"Unknown action: {args.action}")
        return 1
    action_func = actions[args.action]
    result = action_func(args.preset)
    if result != 0:
        log.error(f"Action '{args.action}' failed with exit code: {result}")
    else:
        log.info(f"Action '{args.action}' completed with result: {result}")
    return result


if __name__ == "__main__":
    exit(main())
